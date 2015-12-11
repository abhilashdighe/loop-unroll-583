import csv
import cPickle as pickle
import numpy as np
from sklearn.svm import SVC
from sklearn import metrics

split_data = pickle.load(open('split_data.p'))

X_train = np.array(split_data['X_train'] , dtype=float)
y_train = np.array(split_data['y_train'] , dtype=float)
X_test = np.array(split_data['X_test'] , dtype=float)
y_test = np.array(split_data['y_test'] , dtype=float)



clf = SVC()
clf.fit(X_train , y_train)
preds = clf.predict(X_test)
# fpr, tpr, thresholds = metrics.roc_curve(test_y , preds )
# auc = metrics.auc(fpr , tpr)
accuracy = metrics.accuracy_score(y_test, preds)
print accuracy   

# feature_order = [i[0] for i in sorted(enumerate(clf.features_importances_), key=lambda x:x[1] , reverse=True)]
# print feature_order