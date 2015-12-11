//===-- CustomUnroll.cpp - Loop unroller pass -------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass implements a simple loop unroller.  It works best when loops have
// been canonicalized by the -indvars pass, allowing it to determine the trip
// counts of loops easily.
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "custom-unroll"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/Interval.h"
#include "llvm/Support/CFG.h"
#include "llvm/IR/Instructions.h"
#include "loopLabeler.h"


#include <climits>
#include <map>
#include <fstream>
#include <sstream>
//#include "LAMP/LAMPLoadProfile.h"
//#include "LAMP/LAMPProfiling.h"
//#include "LAMP/label_loop.h"

using namespace llvm;
using namespace std;


static cl::opt<unsigned>UnrollCount("custom-count", cl::init(0), cl::Hidden,
  cl::desc("Use this unroll count for all loops, for testing purposes"));

static cl::opt<bool>UnrollAllowPartial("custom-allow-partial", cl::init(false), cl::Hidden,
  cl::desc("Allows loops to be partially unrolled until "\
           "-custom-threshold loop size is reached."));

static cl::opt<bool>UnrollRuntime("custom-runtime", cl::ZeroOrMore, cl::init(false), cl::Hidden,
  cl::desc("Unroll loops with run-time trip counts"));

static cl::opt<std::string> unrollFacFilename("factors-file",
                        cl::desc("Specify output-filename to read best unroll factors from"), cl::value_desc("filename"));

static cl::opt<std::string> labelInfile("label-infile",
                        cl::desc("Specify label-filename to read loop labels from"), cl::value_desc("filename"));

static cl::opt<unsigned>UnrollThreshold("custom-threshold", cl::init(150), cl::Hidden,
  cl::desc("The cut-off point for automatic loop unrolling"));

namespace {
  class BestUnroll : public LoopPass {
  public:
    static char ID; // Pass ID, replacement for typeid

    map<string, int>* loopIDUnrollFacMap = 0;
    vector<string>* loopIdVec = 0;
    int loopIdCounter = 0;
    BestUnroll(int T = -1, int C = -1,  int P = -1) : LoopPass(ID) {
      CurrentThreshold = (T == -1) ? UnrollThreshold : unsigned(T);
      CurrentAllowPartial = (P == -1) ? UnrollAllowPartial : (bool)P;
      UserThreshold = (T != -1) || (UnrollThreshold.getNumOccurrences() > 0);
      initializeLoopUnrollPass(*PassRegistry::getPassRegistry());
    }

    /// A magic value for use with the Threshold parameter to indicate
    /// that the loop unroll should be performed regardless of how much
    /// code expansion would result.
    static const unsigned NoThreshold = UINT_MAX;

    // Threshold to use when optsize is specified (and there is no
    // explicit -unroll-threshold).
    static const unsigned OptSizeUnrollThreshold = 50;

    // Default unroll count for loops with run-time trip count if
    // -unroll-count is not set
    static const unsigned UnrollRuntimeCount = 8;

    unsigned CurrentThreshold;
    bool     CurrentAllowPartial;
    bool     UserThreshold;        // CurrentThreshold is user-specified.

    bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {

    AU.addRequired<LoopInfo>();
      AU.addPreserved<LoopInfo>();
      AU.addRequiredID(LoopSimplifyID);
      AU.addPreservedID(LoopSimplifyID);
      AU.addRequiredID(LCSSAID);
      AU.addPreservedID(LCSSAID);
      AU.addRequired<ScalarEvolution>();
      AU.addPreserved<ScalarEvolution>();
      AU.addRequired<TargetTransformInfo>();
    // Extra:
    AU.addPreserved<TargetTransformInfo>();

    AU.addRequired<ProfileInfo>();
    // Extra:
    AU.addPreserved<ProfileInfo>();
    //Extra:
    //AU.addPreserved<DominatorTree>();
    }

  private:
    vector<string>* getLoopIdVector();
    map<string, int>* getBestUnrollFactorsMap();

  };
}

vector<string>* BestUnroll :: getLoopIdVector() {
  vector<string> *loopIds = new vector<string>;
  string line;
  ifstream infile(labelInfile.c_str());
  while (getline(infile,line)) {
    loopIds->push_back(line);
  }
  infile.close();
  return loopIds;
}

map<string, int>* BestUnroll :: getBestUnrollFactorsMap() {
  map<string, int> *loopIDUnrollFacMap = new map<string, int>;
  string line;
  ifstream infile(unrollFacFilename.c_str());
  while (getline(infile,line)) {
    string token;
    istringstream ss(line);
    vector<string> v;
    while(getline(ss, token, ',')) {
      v.push_back(token);
    }
    if (v.size() == 2) {
      istringstream buffer(v[1]);
      int unrollFac;
      buffer >> unrollFac;
      (*loopIDUnrollFacMap)[v[0]] = unrollFac;
    }
  }
  infile.close();
  return loopIDUnrollFacMap;
}

char BestUnroll::ID = 0;
static RegisterPass<BestUnroll> M("best-unroll", "Unroll loops");
//INITIALIZE_PASS_BEGIN(CustomUnroll, "custom-unroll", "Unroll loops", false, false)
//INITIALIZE_AG_DEPENDENCY(TargetTransformInfo)
//INITIALIZE_PASS_DEPENDENCY(LoopInfo)
//INITIALIZE_PASS_DEPENDENCY(LoopSimplify)
//INITIALIZE_PASS_DEPENDENCY(LCSSA)
//INITIALIZE_PASS_DEPENDENCY(ScalarEvolution)
//INITIALIZE_PASS_END(CustomUnroll, "custom-unroll", "Unroll loops", false, false)

// Pass* llvm::createLoopUnrollPass(int Threshold, int Count, int AllowPartial) {
//   return new BestUnroll(Threshold, Count, AllowPartial);
// }

/// ApproximateLoopSize - Approximate the size of the loop.
static unsigned ApproximateLoopSize(const Loop *L, unsigned &NumCalls,
                                    bool &NotDuplicatable,
                                    const TargetTransformInfo &TTI) {
  CodeMetrics Metrics;
  for (Loop::block_iterator I = L->block_begin(), E = L->block_end();
       I != E; ++I)
    Metrics.analyzeBasicBlock(*I, TTI);
  NumCalls = Metrics.NumInlineCandidates;
  NotDuplicatable = Metrics.notDuplicatable;

  unsigned LoopSize = Metrics.NumInsts;

  // Don't allow an estimate of size zero.  This would allows unrolling of loops
  // with huge iteration counts, which is a compile time problem even if it's
  // not a problem for code quality.
  if (LoopSize == 0) LoopSize = 1;

  return LoopSize;
}

bool BestUnroll::runOnLoop(Loop *L, LPPassManager &LPM) {

  LoopInfo *LI = &getAnalysis<LoopInfo>();
  ScalarEvolution *SE = &getAnalysis<ScalarEvolution>();
  const TargetTransformInfo &TTI = getAnalysis<TargetTransformInfo>();

  if (!loopIdVec) {
    loopIdVec = getLoopIdVector();
  }

  if(!loopIDUnrollFacMap) {
    loopIDUnrollFacMap = getBestUnrollFactorsMap();
  }

  BasicBlock *Header = L->getHeader();
  DEBUG(dbgs() << "Loop Unroll: F[" << Header->getParent()->getName()
        << "] Loop %" << Header->getName() << "\n");
  (void)Header;

  string loopID = (*loopIdVec)[loopIdCounter];
  loopIdCounter++;

//  errs() << "ID of this loop is " << LP->LoopToIdMap[Header] << "\n";
//  errs() << "ID of this loop is " << LL->LoopToIdMap[Header] << "\n";

  // Determine the current unrolling threshold.  While this is normally set
  // from UnrollThreshold, it is overridden to a smaller value if the current
  // function is marked as optimize-for-size, and the unroll threshold was
  // not user specified.
//  unsigned Threshold = CurrentThreshold;
//  if (!UserThreshold &&
//      Header->getParent()->getAttributes().
//        hasAttribute(AttributeSet::FunctionIndex,
//                     Attribute::OptimizeForSize))
//    Threshold = OptSizeUnrollThreshold;

  // Find trip count and trip multiple if count is not available
  unsigned TripCount = 0;
  unsigned TripMultiple = 1;
  // Find "latch trip count". UnrollLoop assumes that control cannot exit
  // via the loop latch on any iteration prior to TripCount. The loop may exit
  // early via an earlier branch.
  BasicBlock *LatchBlock = L->getLoopLatch();
  if (LatchBlock) {
    errs() << "Assigning tripcount and tripmultiple\n";
    TripCount = SE->getSmallConstantTripCount(L, LatchBlock);
    TripMultiple = SE->getSmallConstantTripMultiple(L, LatchBlock);
  }
  // Use a default unroll-count if the user doesn't specify a value
  // and the trip count is a run-time value.  The default is different
  // for run-time or compile-time trip count loops.

  errs() << "Checking for duplicatable\n";

    unsigned NumInlineCandidates;
    bool notDuplicatable;
    unsigned LoopSize = ApproximateLoopSize(L, NumInlineCandidates,
                                         notDuplicatable, TTI);
    errs() << "Approximate loop size is " << LoopSize << "\n";
    DEBUG(dbgs() << "  Loop Size = " << LoopSize << "\n");
    if (notDuplicatable) {
      DEBUG(dbgs() << "  Not unrolling loop which contains non duplicatable"
            << " instructions.\n");
    errs() << "Not duplicatable!" << "\n";
      return false;
    }

  errs() << "Unrolling the loop\n";

  // Unroll the loop.
  

  // If loopID is not found in map, the loop does not have
  // duplicatable instructions.
  int BestUnrollFactor = 1;
  if(loopIDUnrollFacMap->find(loopID) != loopIDUnrollFacMap->end()){
    BestUnrollFactor = (*loopIDUnrollFacMap)[loopID];
    errs() << "Found best unroll factor of loop! \n";
  }

  if (!UnrollLoop(L, BestUnrollFactor, TripCount, UnrollRuntime, TripMultiple, LI, &LPM)) {
    errs() << "Unrolling the loop failed\n";
    return false;
  } else {
      errs() << "Unrolling loop: " << string(loopID) << " by unroll factor: " << BestUnrollFactor << "\n";
  }

  return false;
}
