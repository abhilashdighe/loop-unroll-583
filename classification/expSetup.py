from sklearn.cross_validation import StratifiedKFold, StratifiedShuffleSplit
from sklearn.grid_search import GridSearchCV
from sklearn.metrics import accuracy_score

def perform_classification(classifier, param_grid,  train_features, y_labels, test_size, iters, num_folds):
    '''
    :param classifier:
    :param param_grid:
    :param train_features:
    :param y_labels:
    :param test_size:
    :param iters:
    :param num_folds:
    :param coeffs:
    :return:
    '''

    iter = 0
    sss = StratifiedShuffleSplit(y_labels, iters, test_size, random_state=None)
    overall_y_pred = []
    overall_y_true = []

    # Stratified splitting of training data into test set and training set
    # When evaluating the resulting model it is important to do it on
    # held-out samples that were not seen during the grid search process:
    # it is recommended to split the data into a development set (to be fed to the GridSearchCV instance)
    # and an evaluation set to compute performance metrics.

    for train_index, test_index in sss:
        train_set = train_features[train_index]
        test_set = train_features[test_index]
        train_labels = y_labels[train_index]
        test_labels = y_labels[test_index]

        # Perform cross validation on train set to tune hyper parameters
        skf_train = StratifiedKFold(train_labels, n_folds=num_folds, shuffle=True, random_state=None)
        clf = GridSearchCV(classifier, param_grid, cv=skf_train)
        clf.fit(train_set, train_labels)

        print ("___Iteration___ ", iter)
        print ("Cross Validation Train Set Report")

        print (clf.best_params_)
        print (clf.best_score_)

        # Fit classifier with opt params on train set
        classifier.set_params(**clf.best_params_)
        classifier.fit(train_set, train_labels)

        y_pred = classifier.predict(test_set)
        overall_y_pred.extend(y_pred.tolist())
        overall_y_true.extend(test_labels.tolist())
        iter += 1

    print ("Test Classification Report")
    # print (classification_report(overall_y_true, overall_y_pred))
    print ("Accuracy: %.3f" % accuracy_score(overall_y_true, overall_y_pred))
    skf_final= StratifiedKFold(y_labels, n_folds=num_folds, shuffle=True, random_state=None)
    clf = GridSearchCV(classifier, param_grid,cv=skf_final)
    clf.fit(train_features, y_labels)
    return clf


