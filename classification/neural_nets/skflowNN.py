import pickle
import skflow
from sklearn import datasets, metrics
import numpy as np

split_data = pickle.load(open('split_data.p'))

X_train = np.array(split_data['X_train'] , dtype=float)
y_train = np.array(split_data['y_train'] , dtype=float)
X_test = np.array(split_data['X_test'] , dtype=float)
y_test = np.array(split_data['y_test'] , dtype=float)

print X_train.shape , y_train.shape
y_train[2]  = 0.
print y_train
print X_train
clf = skflow.TensorFlowDNNClassifier(hidden_units=[10, 20, 10], n_classes=3)
clf.fit(X_train, y_train)
preds = clf.predict(X_test)

score = metrics.accuracy_score(preds, y_test)
print("Accuracy: %f" % score)