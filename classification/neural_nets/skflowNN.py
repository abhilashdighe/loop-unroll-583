import pickle  
import csv 
import sys
import skflow
from sklearn import datasets, metrics
from sklearn.cross_validation import StratifiedKFold
import numpy as np
import matplotlib.pyplot as plt
# split_data = pickle.load(open('split_data.p'))

# X_train = np.array(split_data['X_train'] , dtype=float)
# y_train = np.array(split_data['y_train'] , dtype=float)
# X_test = np.array(split_data['X_test'] , dtype=float)
# y_test = np.array(split_data['y_test'] , dtype=float)
# loopids = split_data['loopids']

def near_accuracy_score(preds, truths):
    total = len(preds)
    correct = 0.0
    for p,t in zip(preds,truths):
        if abs(p-t) < 2:
            correct += 1.0
    return correct / total

def unroll_distance_accuracy(preds , truth):
    distance = []
    for i in range(len(preds)):
        distance.append(abs(preds[i]-truth[i]))

    weights = {}
    for p,t in zip(preds,truth):
        if (p,t) in weights:
            weights[(p,t)] += 1
        else:
            weights[(p,t)] = 1

    x = [p+1 for p,t in sorted(weights.keys())]
    y = [t+1 for p,t in sorted(weights.keys())]
    w = [weights[(p,t)] for p,t in sorted(weights.keys())]

    plt.scatter(x , y , s=w , c='#ff9e00')
    plt.xlabel("Predicted Values")
    plt.ylabel("Optimal Values")
    plt.show()
    return np.mean(distance) , np.std(distance)


def neural_network_classifier(X , y, test_size , iters, num_folds ):
    X_train = X 
    y_train = y    
    # print X_train.shape , y_train.shape

    # print y_train
    # print X_train

    neurons_grid = [5, 10, 15, 20, 25]
    n_classes = 8

    best_accuracy = 0
    best_model = [10,20,10]
    saved_stdout = sys.stdout
    for level1 in neurons_grid:
        for level2 in neurons_grid:
            for level3 in neurons_grid:
                model_accuracy = []
                skf = StratifiedKFold(y_train , n_folds=num_folds)
                for train_index, test_index in skf:
                    X_train_cv, X_test_cv = X_train[train_index], X_train[test_index]
                    y_train_cv, y_test_cv = y_train[train_index], y_train[test_index]
                    sys.stdout = open('trash' , 'w')
                    clf = skflow.TensorFlowDNNClassifier(hidden_units=[level1, level2, level3], n_classes=n_classes)
                    clf.fit(X_train_cv , y_train_cv)
                    preds = clf.predict(X_test_cv)
                    sys.stdout = saved_stdout
                    curr_accuracy = near_accuracy_score(preds,y_test_cv)
                    model_accuracy.append(curr_accuracy)
                    
                model_accuracy = sum(model_accuracy) / len(model_accuracy)
                # print model_accuracy
                if model_accuracy > best_accuracy:
                    best_accuracy = model_accuracy
                    best_model = [level1,level2,level3]
                    print "Better Model Found:" , best_model
    
    print "Accuracy:" , best_accuracy

    overall_y_pred = []
    overall_y_true = []
    skf = StratifiedKFold(y_train , n_folds=num_folds)
    for train_index, test_index in skf:
        X_train_cv, X_test_cv = X_train[train_index], X_train[test_index]
        y_train_cv, y_test_cv = y_train[train_index], y_train[test_index]
        sys.stdout = open('trash' , 'w')
        clf = skflow.TensorFlowDNNClassifier(hidden_units=best_model, n_classes=n_classes)
        clf.fit(X_train_cv , y_train_cv)
        preds = clf.predict(X_test_cv)
        overall_y_pred.extend(preds)
        overall_y_true.extend(y_test_cv)
        sys.stdout = saved_stdout
        # curr_accuracy = metrics.accuracy_score(preds,y_test_cv)
        # model_accuracy.append(curr_accuracy)
    print unroll_distance_accuracy(overall_y_pred , overall_y_true)
    sys.stdout = open('trash' , 'w')
    clf = skflow.TensorFlowDNNClassifier(hidden_units=best_model, n_classes=n_classes)
    clf.fit(X_train, y_train)
    sys.stdout = saved_stdout
    # preds = clf.predict(X_test)
    # sys.stdout = saved_stdout
    # score = metrics.accuracy_score(preds, y_test)

    # print "Best Model:" , best_model
    # print("Accuracy: %f" % score)
    # print "Unroll Factor Distance Accuracy:" , unroll_distance_accuracy(preds , y_test)
    # print zip(preds , y_test)
    # with open('predicted_unroll_factors.csv','w') as predicted_file:
    #     predicted_csv = csv.writer(predicted_file)
    #     for i,loopid in enumerate(loopids):
    #         predicted_csv.writerow([loopid , preds[i]+1])

    # with open('optimal_unroll_factors.csv','w') as optimal_file:
    #     optimal_csv = csv.writer(optimal_file)
    #     for i,loopid in enumerate(loopids):
    #         optimal_csv.writerow([loopid , [i]+1])
    return clf