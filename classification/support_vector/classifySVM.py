import csv
import numpy as np
from sklearn.svm import SVC
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

clf = SVC()
clf.fit(train_x , train_y)
preds = clf.predict(test_x)
# fpr, tpr, thresholds = metrics.roc_curve(test_y , preds )
# auc = metrics.auc(fpr , tpr)
accuracy = metrics.accuracy_score(test_y , preds)
print accuracy   

# feature_order = [i[0] for i in sorted(enumerate(clf.features_importances_), key=lambda x:x[1] , reverse=True)]
# print feature_order