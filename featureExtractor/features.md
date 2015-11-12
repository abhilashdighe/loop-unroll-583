# Features
* Loop nest level
* No. of ops in loop body
  Refers to the size of the loop.
* No. of floating point ops in loop body
* No. of branches in loop body
* No. of memory ops in loop body (have separate features for no. of loads and no. of stores?)
* No. of operands in loop body
* No. of implicit instructions in loop body?
* No. of unique predicates in loop body
* Estimated latency of the critical path of loop?
* Estimated cycle length of loop body?
* No. of parallel “computations” in loop?
* Max dependence height of computations?
* Max height of memory dependencies of computations?
* Max height of control dependencies of computations?
* Average dependence height of computations
* No. of indirect references in loop body
* Min memory-to-memory loop-carried dependence?
* No. of memory-to-memory dependencies
* Tripcount of the loop?
  Number of times the body of the loop is executed.Given that for most of the loops this feature is unknown at compilation time, it has to be determined during the execution of the programs.
* No. of uses in the loop
* No. of defs in the loop
* No. of times the loop is called?
  No. of times the outer loops are executed and the subroutine containing the loop is called.  