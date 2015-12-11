import csv
import sys
import cPickle as pickle
import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from sklearn.svm import SVC, LinearSVC
from skflow import TensorFlowDNNClassifier
from expSetup import perform_classification


def test_logistic_regression(X , y , test_size = 0.3 , iters = 10 , num_folds = 5):
    lr = LogisticRegression()
    grid_params = { 'C':[0.01,0.1,1.0,10,100]}
    clf = perform_classification(lr , grid_params , X , y , test_size , iters, num_folds )


def test_random_forests(X , y , test_size = 0.3 , iters = 5 , num_folds = 5):
    rf = RandomForestClassifier()
    grid_params = { 'n_estimators': [10,20,30],
                    'max_depth': [10,20,30],
                    'min_samples_split': [2 , 3 , 4 ]}
    clf = perform_classification(rf , grid_params , X , y , test_size , iters, num_folds )
    feature_order = [i[0] for i in sorted(enumerate(clf.best_estimator_.feature_importances_), key=lambda x:x[1] , reverse=True)]
    print feature_order


def test_support_vectors(X , y , test_size = 0.3 , iters = 10 , num_folds = 4):
    svm = LinearSVC()
    # grid_params = {'C': [1, 10, 100, 1000], 'kernel': ['linear','rbf']}
                    # {'C': [1, 10, 100, 1000], 'gamma': [0.001, 0.0001], 'kernel': ['rbf']}]
    grid_params = {'C': [1, 10, 100, 1000]}
    clf = perform_classification(svm , grid_params , X , y , test_size , iters, num_folds )


def test_neural_network(X , y , test_size = 0.3 , iters = 10 , num_folds = 5):
    nn = TensorFlowDNNClassifier(hidden_units=[10,10,10] , n_classes=8)
    neuron_grid = [5, 10 , 15 , 20 , 25]
    hidden_units_params = [[n1, n2, n3] for n1 in neuron_grid for n2 in neuron_grid for n3 in neuron_grid]
    print hidden_units_params
    grid_params = { 'hidden_units':hidden_units_params,
                    'n_classes':[8]}
    clf = perform_classification(nn , grid_params , X , y , test_size , iters, num_folds )


def normalize_features(X):
    X_mean = X.mean(axis=0)
    X_std = X.std(axis=0)
    # X_std[X_std==0] = 0.001
    X_norm = (X - X_mean) / X_std
    return X_norm , X_mean , X_std    

data = pickle.load(open('split_data.p'))
X = np.array(data['X'])
y = np.array(data['y'])

X_test = data['X_test']
print len(X_test) , len(X_test[0]), type(X_test), type(X_test[0])
X_test = np.array(X_test)
print X_test.shape
print type(X_test)


# print X.shape
# print np.sum(X[:,8]==0) , X[:,8].shape
# print y[X[:,8]==1]

# print X_test.shape
# print np.sum(X_test[:,8]==1) , X_test[:,8].shape
sys.exit(0)
loopids = data['loopids']

X_norm, X_mean, X_std = normalize_features(X)

if len(sys.argv) < 2:
    print 'Please enter classification command'
    sys.exit(0)

command = sys.argv[1]
if command == 'lr':
    test_logistic_regression(X_norm , y)
elif command == 'rf':
    test_random_forests(X_norm, y)
elif command == 'nn':
    test_neural_network(X_norm, y)
elif command == 'svm':
    test_support_vectors(X_norm, y)
