
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/DataLayout.h" //#include "llvm/Target/TargetData.h"
#include "llvm/ADT/IndexedMap.h"

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "LoopLabeler.h"

using namespace llvm;

void LoopLabelMap::getAnalysisUsage(AnalysisUsage &AU) const {
    AU.setPreservesAll();
}

char LoopLabelMap::ID = 0;
unsigned int LoopLabelMap::loop_id = 0;
static RegisterPass<LoopLabelMap> Y("loop-label","Build the map of Loop Id and Loop");
LoopPass *llvm::createLoopLabelerPass() { return new LoopLabelMap(); }

bool LoopLabelMap::runOnLoop(Loop* L, LPPassManager &LPM)
{
  // build the <IDs, Loop> map
	BasicBlock* BB = L->getHeader();
	loop_id ++;
	
	IdToLoopMap[loop_id] = BB;
	LoopToIdMap[BB] = loop_id;

	llvm::errs() << loop_id << " " << L << " " << BB << " " <<  "\n";
	return true;
}




