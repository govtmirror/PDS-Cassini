/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef TREEENSEMBLE_H
#define TREEENSEMBLE_H


/**
 * <p>
 * This class defined custom types related to random forest and trees.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

#include "common.h"
#include "types.h"
#include "decisiontree.h"
#include "globalvariable.h"
#include "detectorutility.h"


class TreeEnsemble {
private:
    friend class cereal::access;

    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( trees, importances, config);
      }
    public:

    VC<DecisionTree> trees;
    VD importances;
    RandomForestConfig config;

    void clear() {
        trees.clear();
        trees.shrink_to_fit();
    }

    LL countTotalNodes() {
        LL rv = 0;
        REP(i, trees.S) rv += trees[i].nodes.S;
        return rv;
    }

    void printImportances() {
        assert(config.computeImportances);

        VC<pair<double, int>> vp;
        REP(i, importances.S) vp.PB(MP(importances[i], i));
        sort(vp.rbegin(), vp.rend());

        REP(i, importances.S) printf("#%02d: %.6lf\n", vp[i].Y, vp[i].X);
    }

};

class RandomForest : public TreeEnsemble {
    public:

    template <class T> void train(VC<VC<T>> &features, VC<T> &results, RandomForestConfig &_config, int treesMultiplier = 1) {
        //double startTime = startTimeProgram;
        config = _config;

        int treesNo = treesMultiplier * config.treesNo;

        if (config.threadsNo == 1) {
            REP(i, treesNo) {
                /*if (config.timeLimit && getTime() - startTime > config.timeLimit) {
                    DB(i);
                    break;
                }*/
                trees.PB(DecisionTree(features, results, config, gRNG.next()));
            }
        } else {
            thread *threads = new thread[config.threadsNo];
            mutex mutex;
            REP(i, config.threadsNo)
                threads[i] = thread([&] {
                    while (true) {
                        mutex.lock();
                        int seed = gRNG.next();
                        mutex.unlock();
                        auto tree = DecisionTree(features, results, config, seed);
                        mutex.lock();
                        if ((int)trees.S < treesNo)
                            trees.PB(tree);
                        bool done = (int)trees.S >= treesNo;// || config.timeLimit && getTime() - startTime > config.timeLimit;
                        mutex.unlock();
                        if (done) break;
                    }
                });
            REP(i, config.threadsNo) threads[i].join();
            delete[] threads;
        }

        if (config.computeImportances) {
            importances = VD(features[0].S);
            for (DecisionTree tree : trees)
                REP(i, importances.S)
                    importances[i] += tree.importances[i];
            double sum = 0;
            REP(i, importances.S) sum += importances[i];
            REP(i, importances.S) importances[i] /= sum;
        }
    }

    template <class T> double predict(VC<T> &features) {
        assert(trees.S);

        double sum = 0;
        REP(i, trees.S) sum += trees[i].predict(features);
        return sum / trees.S;
    }

    template <class T> VD predict(VC<VC<T>> &features) {
        assert(trees.S);

        int samplesNo = features.S;

        VD rv(samplesNo);
        if (config.threadsNo == 1) {
            REP(j, samplesNo) {
                REP(i, trees.S) rv[j] += trees[i].predict(features[j]);
                rv[j] /= trees.S;
            }
        } else {
            thread *threads = new thread[config.threadsNo];
            REP(i, config.threadsNo)
                threads[i] = thread([&](int offset) {
                    for (int j = offset; j < samplesNo; j += config.threadsNo) {
                        REP(k, trees.S) rv[j] += trees[k].predict(features[j]);
                        rv[j] /= trees.S;
                    }
                }, i);
            REP(i, config.threadsNo) threads[i].join();
            delete[] threads;
        }
        return rv;
    }

};

#endif // TREEENSEMBLE_H

