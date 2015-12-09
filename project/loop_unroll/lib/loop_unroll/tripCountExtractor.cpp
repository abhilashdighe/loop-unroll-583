/*
 * File: tripCountExtractor.cpp
 *
 * Description:
 *  This is a trip count extractor source file for a library. It should be run as a pass with name "TripCountExtractor". It helps to extract
 *  reuses for arrray elements in a loop.
 */

#define DEBUG_TYPE "tripCountExtractor"
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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
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
#include <algorithm>
#include <climits> 

using namespace llvm;
static cl::opt<std::string> reuse_filename("reuse-filename", cl::desc("Specify output-filename to write features to"), cl::value_desc("filename"));

namespace {
  struct TripCountExtractor: public LoopPass {
    static char ID;
    TripCountExtractor() : LoopPass(ID) {}

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<LoopLabelMap>();
      AU.addRequired<ScalarEvolution>();
    }

  private:
    AliasAnalysis *AA;       // Current AliasAnalysis information
    LoopInfo      *LI;       // Current LoopInfo
    LoopLabelMap* LL;

    // State that is updated as we process loops.
    bool Changed;            // Set to true when we change anything.
    Loop *CurLoop;           // The current loop we are working on...
    AliasSetTracker *CurAST; // AliasSet information for the current loop...
    ScalarEvolution *SE;
    DenseMap<Loop*, AliasSetTracker*> LoopToAliasSetMap;

    bool inSubLoop(BasicBlock *BB) {
      assert(CurLoop->contains(BB) && "Only valid if BB is IN the loop");
      return LI->getLoopFor(BB) != CurLoop;
    }

    bool pointerInvalidatedByLoop(Value *V, uint64_t Size, const MDNode *TBAAInfo) {
      // Check to see if any of the basic blocks in CurLoop invalidate *V.
      return CurAST->getAliasSetForPointer(V, Size, TBAAInfo).isMod();
    }

    int getTripCount();
  };
}

char TripCountExtractor::ID = 0;
static RegisterPass<TripCountExtractor> X("tripCountExtractor", "Trip Count Extractor", false, false);

bool TripCountExtractor::runOnLoop(Loop *L, LPPassManager &LPM) {
  Changed = false;

  // Get our Loop and Alias Analysis information
  LI = &getAnalysis<LoopInfo>();
  AA = &getAnalysis<AliasAnalysis>();
  LL = &getAnalysis<LoopLabelMap>();
  SE = &getAnalysis<ScalarEvolution>();

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

  std::string unique_loop_id = LL->LoopToIdMap[CurLoop->getHeader()];
  int reuses = getTripCount();
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

// Pal TODO: Utility function, where to move?
// trip count is the minimum no. of times a loop executes
int TripCountExtractor::getTripCount() {
  if (BasicBlock *ExitingBB = CurLoop->getExitingBlock()) {
    return SE->getSmallConstantTripMultiple(CurLoop, ExitingBB);
  }
  int min = INT_MAX;
  SmallVector<BasicBlock *, 8> UniqueExitBlocks;
  CurLoop->getUniqueExitBlocks(UniqueExitBlocks);
  for (unsigned int i = 0; i < UniqueExitBlocks.size(); i++) {
    BasicBlock *Exit = UniqueExitBlocks[i];
    int exitTripCount = SE->getSmallConstantTripMultiple(CurLoop, Exit);
    min = std::min(exitTripCount, min);
  }
  return min;
}
