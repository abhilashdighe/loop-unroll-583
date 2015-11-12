/*
 * File: featureExtractor.cpp
 *
 * Description:
 *  This is a feature extractor source file for a library. It should be run as a pass with name "featsExtractor". It helps to extract
 *  features for a loop.
 */

#define DEBUG_TYPE "featsExtractor"
#include "llvm/Transforms/Scalar.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AliasSetTracker.h"
#include "llvm/Analysis/ConstantFolding.h"
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


// feature extractor includes
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "LAMP/LAMPLoadProfile.h"
#include "llvm/Analysis/ProfileInfo.h"

// other includes 
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace llvm;

STATISTIC(f1, "Loop nest level");
STATISTIC(f2, "No. of ops in loop body");
STATISTIC(f3, "No. of floating point ops in loop body");
STATISTIC(f4, "No. of branches in loop body");
STATISTIC(f5, "No. of memory ops in loop body");
STATISTIC(f6, "No. of operands in loop body");
STATISTIC(f7, "No. of implicit instructions in loop body");
STATISTIC(f8, "No. of unique predicates in loop body");
STATISTIC(f9, "Estimated latency of the critical path of loop");
STATISTIC(f10, "Estimated cycle length of loop body");
STATISTIC(f11, "No. of parallel computations in loop");
STATISTIC(f12, "Max dependence height of computations");
STATISTIC(f13, "Max height of memory dependencies of computations");
STATISTIC(f14, "Max height of control dependencies of computations");
STATISTIC(f15, "Average dependence height of computations");
STATISTIC(f16, "No. of indirect references in loop body");
STATISTIC(f17, "Min memory-to-memory loop-carried dependence");
STATISTIC(f18, "No. of memory-to-memory dependencies");
STATISTIC(f19, "Tripcount of the loop");
STATISTIC(f20, "No. of uses in the loop");
STATISTIC(f21, "No. of defs in the loop");
STATISTIC(f22, "No. of times the loop is called");


namespace {
  struct FeatureExtractor: public LoopPass {
    static char ID;
    FeatureExtractor() : LoopPass(ID) {}

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<DominatorTree>();
      AU.addRequired<LoopInfo>();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<TargetLibraryInfo>();
      AU.addRequired<ProfileInfo>();
      AU.addRequired<LAMPLoadProfile>();
    }

  private:
    AliasAnalysis *AA;       // Current AliasAnalysis information
    LoopInfo      *LI;       // Current LoopInfo
    DominatorTree *DT;       // Dominator Tree for the current Loop.

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

    unsigned int getLoopDepth();
    std::vector<Instruction*> getDynOps();
    int getFloatOpCount(std::vector<Instruction*> instructions);
    int getBranchOpCount(std::vector<Instruction*> instructions);
    int getMemOpCount(std::vector<Instruction*> instructions);
    int getOperandsCount(std::vector<Instruction*> instructions);
  };
}  

char FeatureExtractor::ID = 0;
static RegisterPass<FeatureExtractor> X("featsExtractor", "Feature Extractor", false, false);

bool FeatureExtractor::runOnLoop(Loop *L, LPPassManager &LPM) {
  Changed = false;

  // Get our Loop and Alias Analysis information...
  LI = &getAnalysis<LoopInfo>();
  AA = &getAnalysis<AliasAnalysis>();
  DT = &getAnalysis<DominatorTree>();

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
  f1 = getLoopDepth();
  f2 = instructions.size();
  f3 = getFloatOpCount(instructions);
  f4 = getBranchOpCount(instructions);
  f5 = getMemOpCount(instructions);
  f6 = getOperandsCount(instructions);

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

unsigned int FeatureExtractor::getLoopDepth() {
  return CurLoop->getLoopDepth();
}

std::vector<Instruction*> FeatureExtractor::getDynOps() {
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

int FeatureExtractor::getFloatOpCount(std::vector<Instruction*> instructions) {
  int floatOpCount = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    switch (I->getOpcode()) {
      case Instruction::FAdd:
      case Instruction::FSub:
      case Instruction::FMul:
      case Instruction::FDiv:
      case Instruction::FRem:
      case Instruction::FCmp:
        floatOpCount += 1;
        break;
      default:
        break;        
    }
  }
  return floatOpCount;
}

int FeatureExtractor::getBranchOpCount(std::vector<Instruction*> instructions) {
  int branchOpCount = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    switch (I->getOpcode()) {
      case Instruction::Br:
      case Instruction::Switch:
      case Instruction::IndirectBr:
        branchOpCount += 1;
        break;
      default:
        break;        
    }
  }
  return branchOpCount;
}

int FeatureExtractor::getMemOpCount(std::vector<Instruction*> instructions) {
  int memOpCount = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    switch (I->getOpcode()) {
      case Instruction::Alloca:
      case Instruction::Load:
      case Instruction::Store:
      case Instruction::GetElementPtr:
      case Instruction::Fence:
      case Instruction::AtomicCmpXchg:
      case Instruction::AtomicRMW:
        memOpCount += 1;
        break;
      default:
        break;        
    }
  }
  return memOpCount;
}

int FeatureExtractor::getOperandsCount(std::vector<Instruction*> instructions) {
  int operandsCount = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    operandsCount += I->getNumOperands();
  }
  return operandsCount;
}
