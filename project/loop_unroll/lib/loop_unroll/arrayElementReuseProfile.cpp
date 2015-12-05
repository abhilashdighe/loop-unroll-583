/*
 * File: arrayElementReuseProfile.cpp
 *
 * Description:
 *  This is a array element reuse profile source file for a library. It should be run as a pass with name "arrayElemReuseProfile". It helps to extract
 *  reuses for arrray elements in a loop.
 */

#define DEBUG_TYPE "arrayElementReuseProfile"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/SSAUpdater.h"

#include <algorithm>


// Feature extractor includes
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "loopLabeler.h" 

// Other includes 
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace llvm;

static cl::opt<std::string> reuse_filename("reuse-filename", cl::desc("Specify output-filename to write features to"), cl::value_desc("filename"));

namespace {
  struct ArrayElementReuseProfile: public LoopPass {
    static char ID;
    ArrayElementReuseProfile() : LoopPass(ID) {}

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTree>();
      AU.addRequired<LoopInfo>();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<TargetLibraryInfo>();
      AU.addRequired<LoopLabelMap>();
    }

  public: 
    std::map<std::string, int> reusesMap;

  private:
    AliasAnalysis *AA;       // Current AliasAnalysis information
    LoopInfo      *LI;       // Current LoopInfo
    DominatorTree *DT;       // Dominator Tree for the current Loop.
    LoopLabelMap* LL;

    DataLayout *TD;          // DataLayout for constant folding.
    TargetLibraryInfo *TLI;  // TargetLibraryInfo for constant folding.

    // State that is updated as we process loops.
    bool Changed;            // Set to true when we change anything.
    Loop *CurLoop;           // The current loop we are working on...
    AliasSetTracker *CurAST; // AliasSet information for the current loop...
    DenseMap<Loop*, AliasSetTracker*> LoopToAliasSetMap;

    bool inSubLoop(BasicBlock *BB) {
      assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
      return LI->getLoopFor(BB) != CurLoop;
    }

    bool pointerInvalidatedByLoop(Value *V, uint64_t Size, const MDNode *TBAAInfo) {
      // Check to see if any of the basic blocks in CurLoop invalidate *V.
      return CurAST->getAliasSetForPointer(V, Size, TBAAInfo).isMod();
    }

    std::vector<Instruction*> getDynOps();
    int getArrayReuses(std::vector<Instruction*> instructions);

  };
}

char ArrayElementReuseProfile::ID = 0;
static RegisterPass<ArrayElementReuseProfile> X("arrayElemReuseProfile", "Array Element Reuse", false, false);

bool ArrayElementReuseProfile::runOnLoop(Loop *L, LPPassManager &LPM) {
  Changed = false;

  // Get our Loop and Alias Analysis information
  LI = &getAnalysis<LoopInfo>();
  AA = &getAnalysis<AliasAnalysis>();
  DT = &getAnalysis<DominatorTree>();
  LL = &getAnalysis<LoopLabelMap>();

  TD = getAnalysisIfAvailable<DataLayout>();
  TLI = &getAnalysis<TargetLibraryInfo>();

  CurAST = new AliasSetTracker(*AA);
  // Collect Alias info from subloops.
  for (Loop::iterator LoopItr = L->begin(), LoopItrE = L->end();
     LoopItr != LoopItrE; ++LoopItr) {
    Loop *InnerL = *LoopItr;
    AliasSetTracker *InnerAST = LoopToAliasSetMap[InnerL];
    assert(InnerAST && "Where is my AST?");
    // What if InnerLoop was modified by other passes ?
    CurAST->add(*InnerAST);

    // Once we've incorporated the inner loop's AST into ours, we don't need the
    // subloop's anymore.
    delete InnerAST;
    LoopToAliasSetMap.erase(InnerL);
  }

  CurLoop = L;
  // Loop over the body of this loop, looking for calls, invokes, and stores.
  // Because subloops have already been incorporated into AST, we skip blocks in
  // subloops.
  //
  for (Loop::block_iterator I = L->block_begin(), E = L->block_end();
     I != E; ++I) {
    BasicBlock *BB = *I;
    if (LI->getLoopFor(BB) == L) {        // Ignore blocks in subloops.
      CurAST->add(*BB);                 // Incorporate the specified basic block
    }
  }

  std::vector<Instruction*> instructions = getDynOps();
  int reuses = getArrayReuses(instructions);
  std::string unique_loop_id = LL->LoopToIdMap[CurLoop->getHeader()];
  reusesMap[unique_loop_id] = reuses;
  std::ofstream outputFile(reuse_filename.c_str(), std::fstream::app);
  outputFile << unique_loop_id << "," << reuses << "\n";
  outputFile.close();

  // Clear out loops state information for the next iteration
  CurLoop = 0;

  // If this loop is nested inside of another one, save the alias information
  // for when we process the outer loop.
  if (L->getParentLoop())
    LoopToAliasSetMap[L] = CurAST;
  else
    delete CurAST;
  return Changed;
}

std::vector<Instruction*> ArrayElementReuseProfile::getDynOps() {
  std::vector<Instruction*> instructions;
  for (Loop::block_iterator I = CurLoop->block_begin(), E = CurLoop->block_end(); I != E; ++I) {
    BasicBlock *BB = *I;
    if (BB == CurLoop->getHeader()) {
      continue;
    }
    if (BB == CurLoop->getLoopLatch()) {
      continue;
    }
    if (LI->getLoopFor(BB) == CurLoop) {        // Ignore blocks in subloops.
      for (BasicBlock::iterator II = BB->begin(), IE = BB->end(); II != IE; ++II) {
        instructions.push_back(II);
      }
    }
  }
  return instructions;
}

// Pal TODO: Utility function, where to move?
int traceArrayReuses(Value* operand) {
  if(isa<Instruction>(operand)) {
    Instruction *I = dyn_cast<Instruction>(operand);
    if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(I)) {
      Value* gepFirstOperand = gep->getOperand(0);
      Type* type = gepFirstOperand->getType();
      // Figure out whether the first operand points to an array
      if (PointerType *pointerType = dyn_cast<PointerType>(type)) {
        Type* elementType = pointerType->getElementType();
        if (elementType->isArrayTy()) {
          return 1;
        }
      }
    } else {
      int arrayReuses = 0;
      for (User::op_iterator OI = I->op_begin(), e = I->op_end(); OI != e; ++OI) {
        arrayReuses += traceArrayReuses(*OI);
      }
      return arrayReuses;
    }
  }
  return 0;
}

// Pal TODO: Explore if we can make it work for a case like:
// x = a[i + 1]
// a[i] = x;
int ArrayElementReuseProfile::getArrayReuses(std::vector<Instruction*> instructions) {
  int arrayReuses = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    if (I->getOpcode() == Instruction::Store) {
      StoreInst *storeInst = dyn_cast<StoreInst>(I);
      Value* target = storeInst->getPointerOperand();
      Value* source = storeInst->getValueOperand();
      if(isa<Instruction>(target)) {
        if(Instruction *opI = dyn_cast<Instruction>(target)) {
          // Check destination, if GEP inst, means is an array el
          if (dyn_cast<GetElementPtrInst>(opI)) {
            // Back track the source to see if it reuses an array el
            arrayReuses += traceArrayReuses(source);
          }
        }
      }
    }
  }
  return arrayReuses;
}
