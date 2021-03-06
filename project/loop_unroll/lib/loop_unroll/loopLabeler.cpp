
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
#include "llvm/Support/CommandLine.h"

#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include "loopLabeler.h"

using namespace llvm;

static cl::opt<std::string> benchmark_name("benchmark", cl::desc("Specify benchmark name filename for loop labeler"), cl::value_desc("filename"));

static cl::opt<std::string> outfile_name("label-outfile", cl::desc("Specify outfile name for loop labeler"), cl::value_desc("outfile"));

void LoopLabelMap::getAnalysisUsage(AnalysisUsage &AU) const {
	AU.setPreservesCFG();
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
	benchmark = benchmark_name;

	std::ostringstream oss;
	oss << benchmark_name << "_" << loop_id;
	std::string unique_loop_id = oss.str();

	IdToLoopMap[unique_loop_id] = BB;
	LoopToIdMap[BB] = unique_loop_id;
	
	llvm::errs() << unique_loop_id << " " << L << " " << BB << " " <<  "\n";

	if (outfile_name.c_str()) {
		std::ofstream outputFile(outfile_name.c_str(), std::fstream::app);
  		outputFile << unique_loop_id << "\n";
  		outputFile.close();
	}
	return false;
}
