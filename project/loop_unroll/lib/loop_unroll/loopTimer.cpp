#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
using namespace llvm;

namespace {
	struct LoopInstrumentation : public ModulePass {
		static char ID;
		LoopInstrumentation() : ModulePass(ID) {}

		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		}

		virtual  bool runOnModule(Module &M) {
			return false;
		}
	};
}

char LoopInstrumentation::ID = 0;
static RegisterPass<LoopInstrumentation> X("loop-timer", "Loop Timing Pass", false, false);

