import csv
import sys
from sklearn.cross_validation import train_test_split
import cPickle as pickle

results_path = "../results/"
benchmarks_complete = ['401.bzip2', '429.mcf', '456.hmmer' , '458.sjeng' , '462.libquantum' , '470.lbm' ]
unroll_factors_range = range(1,9)

loop_to_features = {}
loop_to_timings = {}
for benchmark_name in benchmarks_complete:

    with open(results_path+benchmark_name+'_train.txt') as features_file:
        features_csv = csv.reader(features_file)
        head = features_csv.next()
        for sample in features_csv:
            benchmark , loopid = sample[:2]
            features = map(float , sample[2:])
            # if features[-1] == 1:
            #     continue
            loop_to_features[loopid] = features
    
    for unroll_factor in unroll_factors_range:
        extension = '\.csv.csv'
        if benchmark_name == '470.lbm':
            extension = '\.txt.csv'

        with open(results_path+benchmark_name+'_train'+str(unroll_factor)+extension) as runtime_file:
            runtimes_csv = csv.reader(runtime_file)
            head = runtimes_csv.next()
            for run in runtimes_csv:
                loopid , time = run
                # print benchmark , loopid , trip_count , unroll_factor , time
                # time_per_trip = float(time) / float(trip_count)
                time=float(time)
                if loopid in loop_to_timings:
                    if unroll_factor in loop_to_timings[loopid]:
                        # print loop_to_timings[(benchmark,loopid)]
                        (loop_to_timings[loopid])[unroll_factor].append(time)
                    else:
                        # print loop_to_timidngs[(benchmark,loopid)]
                        (loop_to_timings[loopid])[unroll_factor] = [time,]
                else:
                    loop_to_timings[loopid] = {}
                    loop_to_timings[loopid][unroll_factor] = [time,]

loop_to_label = {}
for loopid in loop_to_timings:
    
    best_factor = -1
    best_time = sys.maxint
    for unroll_factor in loop_to_timings[loopid]:
        curr_timing_list = loop_to_timings[loopid][unroll_factor]
        curr_time = sum(curr_timing_list) / len(curr_timing_list)
        loop_to_timings[loopid][unroll_factor] = curr_time
        if curr_time < best_time:
            best_time = curr_time
            best_factor = unroll_factor

    loop_to_label[loopid] = best_factor


X = []
y = []
loopids = []

with open('labelled_data.csv' , 'wb') as labelled_file:
    labelled_csv = csv.writer(labelled_file)
    for loopid in sorted(loop_to_features.keys()):
        if loopid in loop_to_timings:
            features = loop_to_features[loopid]
            X.append(features)
            label = loop_to_label[loopid]
            y.append(int(label)-1)
            features.append(label)
            labelled_csv.writerow(features)
            loopids.append(loopid)

with open('optimal_unroll_factors.csv','w') as optimal_file:
    optimal_csv = csv.writer(optimal_file)
    for i,loopid in enumerate(loopids):
        optimal_csv.writerow([loopid , y[i]+1])


X_train, X_test, y_train, y_test = train_test_split( X, y, test_size=0.33, random_state=0 , stratify=y)

print y_train
print y_test
data = {}
data['X'] = X
data['y'] = y
data['X_train'] = X_train
data['X_test'] = X_test
data['y_train'] = y_train
data['y_test'] = y_test
data['loopids'] = loopids
pickle.dump( data , open('split_data.p' , 'wb'))