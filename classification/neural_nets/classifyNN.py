import csv
import numpy as np
from sklearn.neural_network import MLPClassifier
from sklearn import metrics


train_x = []
train_y = []
    
test_x = []
test_y = []

with open('data.train') as train_file:
    train_csv = csv.reader(train_file ,delimiter=' ')
    for sample in train_csv:
        sample = map(float , sample)
        train_x.append(sample[:-1])
        train_y.append(sample[-1])

with open('data.test') as test_file:
    test_csv = csv.reader(test_file , delimiter=' ')
    for sample in test_csv:
        sample = map(float , sample)
        test_x.append(sample[:-1])
        test_y.append(sample[-1])

clf = MLPClassifier()
clf.fit(train_x , train_y)
preds = clf.predict(test_x)
# fpr, tpr, thresholds = metrics.roc_curve(test_y , preds )
# auc = metrics.auc(fpr , tpr)
accuracy = metrics.accuracy_score(test_y , preds)
print accuracy   