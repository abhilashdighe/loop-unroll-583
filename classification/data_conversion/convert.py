import csv
import sys
from sklearn.cross_validation import train_test_split
import cPickle as pickle

loop_to_features = {}
loop_to_timings = {}
with open('features.csv') as features_file:
    features_csv = csv.reader(features_file)
    for sample in features_csv:
        benchmark , loopid = sample[:2]
        features = map(float , sample[2:])
        loop_to_features[(benchmark,loopid)] = features

with open('runtime.csv') as runtime_file:
    runtimes_csv = csv.reader(runtime_file)
    for sample in runtimes_csv:
        loopid , trip_count , unroll_factor , time = sample
        # print benchmark , loopid , trip_count , unroll_factor , time
        # time_per_trip = float(time) / float(trip_count)
        time=float(time)
        if loopid in loop_to_timings:
            if unroll_factor in loop_to_timings[loopid]:
                # print loop_to_timings[(benchmark,loopid)]
                (loop_to_timings[loopid])[unroll_factor].append(time)
            else:
                # print loop_to_timings[(benchmark,loopid)]
                (loop_to_timings[loopid])[unroll_factor] = [time]
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
with open('labelled_data.csv' , 'wb') as labelled_file:
    labelled_csv = csv.writer(labelled_file)
    for benchmark , loopid in loop_to_features:
        if loopid in loop_to_timings:
            features = loop_to_features[(benchmark,loopid)]
            X.append(features)
            label = loop_to_label[loopid]
            y.append(int(label)-1)
            features.append(label)
            labelled_csv.writerow(features)


X_train, X_test, y_train, y_test = train_test_split( X, y, test_size=0.33, random_state=42 , stratify=y)

print y_train
print y_test
data = {}
data['X_train'] = X_train
data['X_test'] = X_test
data['y_train'] = y_train
data['y_test'] = y_test

pickle.dump( data , open('split_data.p' , 'wb'))