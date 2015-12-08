import csv
import pickle
import numpy as np
from sklearn.linear_model import LogisticRegressionCV , LogisticRegression
from sklearn import metrics

split_data = pickle.load(open('split_data.p'))

X_train = np.array(split_data['X_train'] , dtype=float)
y_train = np.array(split_data['y_train'] , dtype=float)
X_test = np.array(split_data['X_test'] , dtype=float)
y_test = np.array(split_data['y_test'] , dtype=float)


# C_grid = [0.5 , 1.0 , 5 , 10]
# skf = StratifiedKFold(y_test, n_folds = 3)
# max_auc = 0
# best_C = 0.1
# for C in C_grid:
#     # clf = Lasso(alpha=1)
#     curr_aucs = []
#     for train_index , test_index in skf:
#         clf = LogisticRegression(C=C)   
#         clf.fit(X_train[train_index,:] , y_train[train_index])
#         preds = clf.predict_proba(X_train[test_index ,:])
#         fpr, tpr, thresholds = metrics.roc_curve(X_train[test_index] , preds[:,1], pos_label=1)
#         auc = metrics.auc(fpr , tpr)
#         if auc < 0.5:
#             auc = 1 - auc
#         curr_aucs.append(auc)
#     auc_avg = sum(curr_aucs)/len(curr_aucs)
#     # print C , auc_avg
#     if auc_avg > max_auc:
#         best_C = C
#         max_auc = auc_avg
# clf = LogisticRegression(C=best_C)


clf = LogisticRegressionCV()


clf.fit(X_train , y_train)
preds = clf.predict(X_test)
# fpr, tpr, thresholds = metrics.roc_curve(test_y , preds )
# auc = metrics.auc(fpr , tpr)
accuracy = metrics.accuracy_score(y_test, preds)
print accuracy   

feature_order = [i[0] for i in sorted(enumerate(abs(clf.coef_[0])), key=lambda x:x[1] , reverse=True)]
print feature_order