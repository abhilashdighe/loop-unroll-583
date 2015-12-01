# TODO list + Data Flow Specs

### Framework
##### Create files
- train_x.csv: This will contain entries for <benchmark, loop_id, feature_1, feature_2....>
- train_times.csv: This will contain entries for <benchmark, loop_id, unroll_factor, execution_time>
- train_y.csv: This will find out the unroll_factor that gives best execution_time. This will contain entires like <benchmark, loop_id, best_unroll_factor>
- train.csv: Join train_x.csv and train_y.csv
- test_x.csv: This will contain entries for <benchmark, loop_id, feature_1, feature_2....>
- test_y.csv: This will be the output of the machine learning algorithm. This will cotnain entries like <benchmark, loop_id, predicted_unroll_factor>
- test_times.csv: This will contain the execution times of test, once the predicted unroll factors are applied. Since this does not require loop-level instrumentation, should just contain entries like <benchmark, execution_time>
- test_baseline_times.csv: This will contain the baseline execution times of test. Should contain entries like <benchmark, execution_time>

##### Loop labeler pass
Assigns unique loop_id to each loop in a benchmark. To uniquely identify loops across benchmarks, this pass should accept an option 'benchmark_name' and name the loops like 'benchmark_name_#'. 

TODO: add option for benchmark_name

##### Feature extractor pass
Extracts features for train/test benchmarks. Should have a dependency on loop labeler pass. Uses loop id defined by the loop labeler and extract features for a loop. Should write its output to 'train_x.csv' / 'test_x.csv'

TODO: 1) fix the bug with feature that computes array element reuses. 2) add option for file to append features to (other option could be to hard code/redirect output of the pass, but that will require script to merge the outputs)

##### Loop Unroll pass
Should have a dependency on loop labeler pass.
For Training: Unrolls all the loops in a benchmark to a given unroll factor (1..8). For convention, name the unrolled bitcode files as 'benchmark_name_<unroll_factor>.bc'. These files will be consumed by loop timer pass for loop level instrumentation.
For Testing: Should be able to read the loop unroll factors from a given file and unroll a loop to its corresponding unroll factor. Generates 'benchmark_name_optimal_unroll.bc' which will then be executed directly.

TODO: if the above tasks need to achieved using scripts, write appropriate scripts.

##### Loop Timer pass
Does loop level instrumentation. Should have a dependency on loop labeler pass. Will be used for training phase only.
Should take 'benchmark_name_<unroll_factor>.bc' and instrument it at loop level. Should write its output to 'train_times.csv'

TODO: complete the implementation

##### Script/Program for handling files

1) write a script/program that takes 'train_times.csv' as input and generates 'train_y.csv'
2) write a script/program that combines 'train_x.csv' and 'train_y.csv'. If the machine learning algorithm can deal with separate inputs files, this step is not required.

##### Machine learning algorithm(s)

Should take as input 'train.csv' and 'test_x.csv' and give 'test_y.csv'. It wil be nice if it can provide most informative features. 

TODO: check if all of this is in place.

##### Script/program for test benchmarks

1) write a script that will use 'test_y.csv', will do loop-unrolling and execute the benchmarks. Should generate 'test_times.csv'
2) write a script that will simply execute the benchmarks for baseline. Should generate 'test_baseline_times.csv'

##### Server set up

Set up a server accessible to the team to run the experiments. It will be nice if the scripts on the github are specific to the server set up and not local environments.

### Deliverables

- Graphs to compare performance with baseline
- Figures to visualize most informative features or figures to visualize decision tree etc.
- Written report
-- related work + approach + experiments + evaluation
- Final presentation

-- Due date: Thursday, Dec 10, 2015, 11:59 PM
