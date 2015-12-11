import csv
import cPickle as pickle
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn import metrics


def unroll_distance_accuracy(preds , truth):
    distance = []
    for i in range(len(preds)):
        distance.append(abs(preds[i]-truth[i]))

    return np.mean(distance) , np.std(distance)

split_data = pickle.load(open('split_data.p'))

X_train = np.array(split_data['X_train'] , dtype=float)
y_train = np.array(split_data['y_train'] , dtype=float)
X_test = np.array(split_data['X_test'] , dtype=float)
y_test = np.array(split_data['y_test'] , dtype=float)



clf = RandomForestClassifier(n_estimators=10, max_depth=100)
clf.fit(X_train , y_train)
preds = clf.predict(X_test)
# fpr, tpr, thresholds = metrics.roc_curve(test_y , preds )
# auc = metrics.auc(fpr , tpr)
accuracy = metrics.accuracy_score(y_test , preds)
print accuracy   

feature_order = [i[0] for i in sorted(enumerate(clf.feature_importances_), key=lambda x:x[1] , reverse=True)]
print feature_order

print "Unroll Factor Distance Accuracy:" , unroll_distance_accuracy(preds , y_test)
print zip(preds , y_test)
