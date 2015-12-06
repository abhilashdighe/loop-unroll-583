import pickle
import sys
import skflow
from sklearn import datasets, metrics
from sklearn.cross_validation import StratifiedKFold
import numpy as np

split_data = pickle.load(open('split_data.p'))

X_train = np.array(split_data['X_train'] , dtype=float)
y_train = np.array(split_data['y_train'] , dtype=float)
X_test = np.array(split_data['X_test'] , dtype=float)
y_test = np.array(split_data['y_test'] , dtype=float)

print X_train.shape , y_train.shape
y_train[2]  = 0.
# print y_train
# print X_train

neurons_grid = [5, 10, 15, 20, 25]


best_accuracy = 0
best_model = [10,20,10]
saved_stdout = sys.stdout
for level1 in neurons_grid:
    for level2 in neurons_grid:
        for level3 in neurons_grid:
            model_accuracy = []
            skf = StratifiedKFold(y_train , n_folds=2)
            for train_index, test_index in skf:
                X_train_cv, X_test_cv = X_train[train_index], X_train[test_index]
                y_train_cv, y_test_cv = y_train[train_index], y_train[test_index]
                sys.stdout = open('trash' , 'w')
                clf = skflow.TensorFlowDNNClassifier(hidden_units=[level1, level2, level3], n_classes=3)
                clf.fit(X_train_cv , y_train_cv)
                preds = clf.predict(X_test_cv)
                sys.stdout = saved_stdout
                curr_accuracy = metrics.accuracy_score(preds,y_test_cv)
                model_accuracy.append(curr_accuracy)
                
            model_accuracy = sum(model_accuracy) / len(model_accuracy)
            # print model_accuracy
            if model_accuracy > best_accuracy:
                best_accuracy = model_accuracy
                best_model = [level1,level2,level3]
                print "Better Model Found:" , best_model

sys.stdout = open('trash' , 'w')
clf = skflow.TensorFlowDNNClassifier(hidden_units=best_model, n_classes=3)
clf.fit(X_train, y_train)
preds = clf.predict(X_test)
sys.stdout = saved_stdout

score = metrics.accuracy_score(preds, y_test)

print "Best Model:" , best_model
print("Accuracy: %f" % score)