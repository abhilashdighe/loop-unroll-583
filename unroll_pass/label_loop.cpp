
#include "/opt/llvm-source/include/llvm/IR/Module.h"
#include "/opt/llvm-source/include/llvm/Pass.h"
#include "/opt/llvm-source/include/llvm/Analysis/Passes.h"
#include "/opt/llvm-source/include/llvm/Analysis/LoopPass.h"
#include "/opt/llvm-source/include/llvm/Analysis/LoopInfo.h"
#include "/opt/llvm-source/include/llvm/IR/Constants.h"
#include "/opt/llvm-source/include/llvm/IR/Instructions.h"
#include "/opt/llvm-source/include/llvm/IR/Function.h"
#include "/opt/llvm-source/include/llvm/IR/BasicBlock.h"
#include "/opt/llvm-source/include/llvm/Support/Debug.h"
#include "/opt/llvm-source/include/llvm/IR/DataLayout.h" //#include "llvm/Target/TargetData.h"
#include "/opt/llvm-source/include/llvm/ADT/IndexedMap.h"
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include "LAMP/label_loop.h"

using namespace llvm;


static RegisterPass<label_loop> M("label-loop","Build the map of Loop Id and Loop Header pointer");
//LoopPass *llvm::createLabelLoopPass() { return new label_loop(); }

bool label_loop::runOnLoop(Loop* L, LPPassManager &LPM)
{
  // build the <IDs, Loop> map
	BasicBlock* BB = L->getHeader();
	loop_id ++;

	IdToLoopMap[loop_id] = BB;
	LoopToIdMap[BB] = loop_id;

	llvm::errs() << loop_id << " " << L << " " << BB << " " << /* IdToLoopMap_global.size() <<*/ "\n";
  
	return true;
}


