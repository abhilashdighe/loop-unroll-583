//===- LoopLabeler.h - Instrumentation passes --*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// Info...
//
//===----------------------------------------------------------------------===//
#ifndef LOOPLABELER_H
#define LOOPLABELER_H

namespace llvm {

  class LoopPass;

  LoopPass *createLoopLabelerPass();

  class LoopLabelMap : public LoopPass {
    static unsigned int loop_id;
    static bool IdInitFlag;
    
  public:
    std::map<unsigned int, BasicBlock*> IdToLoopMap;    // LoopID -> headerBB*
    std::map<BasicBlock*, unsigned int> LoopToIdMap;    // headerBB* -> LoopID
    static char ID;
    LoopLabelMap() : LoopPass(ID) {}

    virtual bool runOnLoop (Loop *L, LPPassManager &LPM);
    virtual void getAnalysisUsage(AnalysisUsage &AU) const;
  };  
}
#endif
 