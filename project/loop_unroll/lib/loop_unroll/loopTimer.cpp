#include "llvm/Analysis/LoopInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "loopLabeler.h"

#include <vector>
#include <string>
using namespace llvm;
using namespace std;

namespace {
	struct ModuleInstrumentation : public ModulePass {
		static char ID;
	public:
		Function *startTimerHook;
		Function *endTimerHook;

		ModuleInstrumentation() : ModulePass(ID) {}
		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
		}

		virtual  bool runOnModule(Module &M) {
			Constant *hookFunc;
			// Start timer hook
            hookFunc = M.getOrInsertFunction("_Z10startTimerPc", \
											 Type::getVoidTy(M.getContext()),
                                             PointerType::get(Type::getInt8Ty(M.getContext()), 0),
											 (Type*)0);
            startTimerHook = cast<Function>(hookFunc);
			// End timer hook
			hookFunc = M.getOrInsertFunction("_Z8endTimerPci", \
											 Type::getVoidTy(M.getContext()),
											 PointerType::get(Type::getInt8Ty(M.getContext()), 0),
											 Type::getInt32Ty(M.getContext()),
											 (Type*)0);
            endTimerHook = cast<Function>(hookFunc);

			return false;
		}
	};
}

namespace {

	struct LoopInstrumentation : public LoopPass {
		static char ID;
		LoopLabelMap* LL;
		std::map<Loop*, AllocaInst*> countersMap;

		LoopInstrumentation() : LoopPass(ID) {}
		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<ModuleInstrumentation>();
			AU.addRequired<LoopLabelMap>();
		}

		virtual  bool runOnLoop(Loop *L, LPPassManager &LPM) {

			LL = &getAnalysis<LoopLabelMap>();
			string loopID = LL->LoopToIdMap[L->getHeader()];

			ModuleInstrumentation *MI = &getAnalysis<ModuleInstrumentation>();
			Function *startTimerHook = MI->startTimerHook;
			Function *endTimerHook = MI->endTimerHook;
			BasicBlock *Preheader = L->getLoopPreheader();

			IRBuilder<> builder(Preheader);

			string loopCounterID = loopID + "_counter";
			AllocaInst *loopCounterVar = new AllocaInst(Type::getInt32Ty(Preheader->getContext()), loopCounterID, Preheader->getTerminator());
			countersMap[L] = loopCounterVar;
			StoreInst *loopCounterInit = new StoreInst(builder.getInt32(0), loopCounterVar);
			loopCounterInit->insertAfter(loopCounterVar);

			BasicBlock *header = L->getHeader();
			BasicBlock *loopLatch = L->getLoopLatch();
			LoadInst *loopCounterVal = new LoadInst(loopCounterVar, loopCounterID + "_val" , loopLatch->getTerminator());

			Value *one =  ConstantInt::get(Type::getInt32Ty(header->getContext()),1);
			BinaryOperator *newInst = BinaryOperator::Create(Instruction::Add, loopCounterVal, one ,loopCounterID + "_new");
			newInst->insertAfter(loopCounterVal);
			StoreInst *loopCounterUpdate = new StoreInst(newInst, loopCounterVar);
			loopCounterUpdate->insertAfter(newInst);

			// Build argument LoopID
			const char *loopIDVal = loopID.c_str();
			Value *loopIDGS = builder.CreateGlobalString(loopIDVal, loopID);
			Value* argLoopID = builder.CreateConstGEP2_32(loopIDGS, 0, 0, "cast");

			// Insert call to startTimer in loop preheader
			Instruction *startTimerInst = CallInst::Create(startTimerHook, argLoopID, "");
			startTimerInst->insertBefore(Preheader->getTerminator());

            // Insert call to endTimer in loop exit block

            SmallVector<BasicBlock *, 8> UniqueExitBlocks;
            L->getUniqueExitBlocks(UniqueExitBlocks);
            for (unsigned int i = 0; i < UniqueExitBlocks.size(); i++) {
            	BasicBlock *Exit = UniqueExitBlocks[i];
            	LoadInst *loopCounterExitVal = new LoadInst(loopCounterVar, loopCounterID + "_exit_val", &(Exit->front()));

            	vector<Value*> args;
				args.push_back(argLoopID);
				args.push_back(loopCounterExitVal);

				Instruction *endTimerInst = CallInst::Create(endTimerHook, ArrayRef<Value*>(args), "");
				endTimerInst->insertAfter(loopCounterExitVal);
            }

			//BasicBlock *Exit = L->getUniqueExitBlock();
			return false;
		}


	};
}

char ModuleInstrumentation::ID = 0;
char LoopInstrumentation::ID = 1;

static RegisterPass<ModuleInstrumentation> X("module-inst", "Loop Timing Module Pass", false, false);
static RegisterPass<LoopInstrumentation> Y("loop-inst", "Loop Timing Loop Pass", false, false);

