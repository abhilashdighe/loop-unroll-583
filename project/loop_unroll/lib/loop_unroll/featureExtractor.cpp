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
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/TargetTransformInfo.h"



#include <algorithm>


// Feature extractor includes
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/ProfileInfo.h"
#include "loopLabeler.h" 

// Other includes 
#include <set>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace llvm;

// STATISTIC(f1, "Loop nest level");
// STATISTIC(f2, "No. of ops in loop body");
// STATISTIC(f3, "No. of floating point ops in loop body");
// STATISTIC(f4, "No. of branches in loop body");
// STATISTIC(f5, "No. of memory ops in loop body");
// STATISTIC(f6, "No. of operands in loop body");
// STATISTIC(f7, "No. of implicit instructions in loop body");
// STATISTIC(f8, "No. of unique predicates in loop body");
// STATISTIC(f9, "Tripcount of the loop");
// STATISTIC(f10, "No. of times the loop is called");
// STATISTIC(f11, "No. of array element reuses");
// STATISTIC(f12, "No. of uses in the loop");
// STATISTIC(f13, "No. of defs in the loop");
// STATISTIC(f14, "No. of indirect array element accesses");
// STATISTIC(f15, "Estimated latency of the critical path of loop");
// STATISTIC(f16, "Estimated cycle length of loop body");
// STATISTIC(f17, "No. of parallel computations in loop");
// STATISTIC(f18, "Max dependence height of computations");
// STATISTIC(f19, "Max height of memory dependencies of computations");
// STATISTIC(f20, "Max height of control dependencies of computations");
// STATISTIC(f21, "Average dependence height of computations");
// STATISTIC(f22, "No. of indirect references in loop body");
// STATISTIC(f23, "Min memory-to-memory loop-carried dependence");
// STATISTIC(f24, "No. of memory-to-memory dependencies");

static cl::opt<std::string> output_filename("output-filename", cl::desc("Specify output-filename to write features to"), cl::value_desc("filename"));
static cl::opt<std::string> tripcount_filename("trip-count-filename", cl::desc("Specify trip_count-filename to read feature from"), cl::value_desc("filename"));

namespace {
  struct FeatureExtractor: public LoopPass {
    static char ID;
    FeatureExtractor() : LoopPass(ID) {}

    virtual bool runOnLoop(Loop *L, LPPassManager &LPM);

    /// This transformation requires natural loop information & requires that
    /// loop preheaders be inserted into the CFG...
    ///
    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
      AU.addRequired<LoopInfo>();
      AU.addRequired<AliasAnalysis>();
      AU.addRequired<ProfileInfo>();
      AU.addRequired<LoopLabelMap>();
      AU.addRequired<TargetTransformInfo>();
    }

  private:
    AliasAnalysis *AA;       // Current AliasAnalysis information
    LoopInfo      *LI;       // Current LoopInfo
    ProfileInfo* PI;
    LoopLabelMap* LL;
    std::map<std::string, int> *tripCountMap = 0;

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
    double getFloatOpCount(std::vector<Instruction*> instructions);
    double getBranchOpCount(std::vector<Instruction*> instructions);
    double getMemOpCount(std::vector<Instruction*> instructions);
    double getOperandsCount(std::vector<Instruction*> instructions);
    double getImplicitInstructionsCount(std::vector<Instruction*> instructions);
    double getUniquePredicatesCount(std::vector<Instruction*> instructions);
    int getLoopCallCount();
    int getArrayReuses(std::vector<Instruction*> instructions);
    double getDefCount(std::vector<Instruction*> instructions);
    double getUseCount(std::vector<Instruction*> instructions);
    double getStoreCount(std::vector<Instruction*> instructions);
    double getLoadCount(std::vector<Instruction*> instructions);
    int notDuplicatable(const TargetTransformInfo &TTI);
    std::map<std::string, int>* getTripCountMap();     
  };
}

char FeatureExtractor::ID = 0;
static RegisterPass<FeatureExtractor> X("featsExtractor", "Feature Extractor", false, false);

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

bool FeatureExtractor::runOnLoop(Loop *L, LPPassManager &LPM) {
  Changed = false;

  // Get our Loop and Alias Analysis information
  LI = &getAnalysis<LoopInfo>();
  AA = &getAnalysis<AliasAnalysis>();
  PI = &getAnalysis<ProfileInfo>();
  LL = &getAnalysis<LoopLabelMap>();
  const TargetTransformInfo &TTI = getAnalysis<TargetTransformInfo>();

  CurAST = new AliasSetTracker(*AA);
  // Collect Alias info from subloops.

  if (!(tripCountMap)) {
    tripCountMap = getTripCountMap();
  }

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
  std::vector<Instruction*> instructions = getDynOps();

  int loop_nest_level = getLoopDepth();
  int dynOp_count = instructions.size();
  double floatOp_count = getFloatOpCount(instructions);
  double branchOp_count = getBranchOpCount(instructions);
  double memOp_count = getMemOpCount(instructions);
  double operands_count = getOperandsCount(instructions);
  double implicit_instr_count = getImplicitInstructionsCount(instructions);
  double unique_pred_count = getUniquePredicatesCount(instructions);
  int trip_count = (*tripCountMap)[unique_loop_id];
  int loop_calls = getLoopCallCount();
  int reuse_count = getArrayReuses(instructions);
  double use_count = getUseCount(instructions);
  double def_count = getDefCount(instructions);
  double store_count = getStoreCount(instructions);
  double load_count = getLoadCount(instructions);
  int isNotDuplicatable = notDuplicatable(TTI);
  std::ofstream outputFile(output_filename.c_str(), std::fstream::app);
  outputFile << LL->benchmark << ","
                << unique_loop_id << ","
                << loop_nest_level << ","
                << dynOp_count << ","
                << floatOp_count << ","
                << branchOp_count << ","
                << memOp_count<< ","
                << operands_count << ","
                << implicit_instr_count << ","
                << unique_pred_count << ","
                << trip_count << ","
                << loop_calls << ","
                << reuse_count << ","
                << use_count << ","
                << def_count << ", "
                << store_count << ", "
                << load_count << ", "
                << isNotDuplicatable << "\n";
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

int FeatureExtractor::notDuplicatable(const TargetTransformInfo &TTI) {
  unsigned NumInlineCandidates;
  bool notDuplicatable;
  ApproximateLoopSize(CurLoop, NumInlineCandidates, notDuplicatable, TTI);
  return notDuplicatable;
}

std::map<std::string, int>* FeatureExtractor::getTripCountMap() {
  std::map<std::string, int> *trip_count_map = new std::map<std::string, int>;
  std::string line;
  std::ifstream infile(tripcount_filename.c_str());
  while (getline(infile,line)) {
    std::string token;
    std::istringstream ss(line);
    std::vector<std::string> v;
    while(std::getline(ss, token, ',')) {
      v.push_back(token);
    }
    if (v.size() == 2) {
      std::istringstream buffer(v[1]);
      int reuses;
      buffer >> reuses;
      (*trip_count_map)[v[0]] = reuses;
    }   
  }
  infile.close();
  return trip_count_map;
}

//return the nesting level of the loop
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

double FeatureExtractor::getFloatOpCount(std::vector<Instruction*> instructions) {
  double floatOpCount = 0.0;
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
  if (instructions.size() > 0) {
    return floatOpCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getBranchOpCount(std::vector<Instruction*> instructions) {
  double branchOpCount = 0;
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
  if (instructions.size() > 0) {
    return branchOpCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getMemOpCount(std::vector<Instruction*> instructions) {
  double memOpCount = 0;
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
  if (instructions.size() > 0) {
    return memOpCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getOperandsCount(std::vector<Instruction*> instructions) {
  double operandsCount = 0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    operandsCount += I->getNumOperands();
  }
  if (instructions.size() > 0) {
    return operandsCount / instructions.size();
  }
  return 0.0;
}

//instructions that are either shift or cast. Should binary op be included?
double FeatureExtractor::getImplicitInstructionsCount(std::vector<Instruction*> instructions) {
  double implictInstrCount = 0.0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    if (Instruction::isShift(I->getOpcode()) || Instruction::isCast(I->getOpcode())) {
      implictInstrCount++;
    }
  }
  if (instructions.size() > 0) {
    return implictInstrCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getUniquePredicatesCount(std::vector<Instruction*> instructions) {
  std::set<CmpInst::Predicate> predicates;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    if (CmpInst *compInst = dyn_cast<CmpInst>(I)) {
      predicates.insert(compInst->getPredicate());
    }
  }
  if (instructions.size() > 0) {
    return predicates.size() * 1.0 / instructions.size();
  }
  return 0.0;
}

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
int FeatureExtractor::getArrayReuses(std::vector<Instruction*> instructions) {
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

// get no. of times the loop is called
int FeatureExtractor::getLoopCallCount() {
  BasicBlock *Preheader = CurLoop->getLoopPreheader();
  if (!Preheader) {
    return 1;
  }
  return PI->getExecutionCount(Preheader);
}

//how many instructions are being used by the instructions in the loop.
double FeatureExtractor::getUseCount(std::vector<Instruction*> instructions) {
  double useCount = 0.0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    for (User::op_iterator OI = I->op_begin(), OE = I->op_end(); OI != OE; ++OI) {
      if(dyn_cast<Instruction>(*OI)) {
        useCount++;
      }
    }
  }
  if (instructions.size() > 0) {
    return useCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getDefCount(std::vector<Instruction*> instructions) {
  double defCount = 0.0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    if (dyn_cast<AllocaInst>(I)) {
      defCount++;
    }
  }
  if (instructions.size() > 0) {
    return defCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getStoreCount(std::vector<Instruction*> instructions) {
  double storeCount = 0.0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    switch (I->getOpcode()) {
    case Instruction::Store:
      storeCount += 1;
      break;
    default:
      break;
    }
  }
  if (instructions.size() > 0) {
    return storeCount / instructions.size();
  }
  return 0.0;
}

double FeatureExtractor::getLoadCount(std::vector<Instruction*> instructions) {
  double loadCount = 0.0;
  for (std::vector<Instruction*>::iterator II = instructions.begin(), IE = instructions.end(); II != IE;  ++II) {
    Instruction *I = *II;
    switch (I->getOpcode()) {
    case Instruction::Load:
      loadCount += 1;
      break;
    default:
      break;
    }
  }
  if (instructions.size() > 0) {
    return loadCount / instructions.size();
  }
  return 0.0;
}
