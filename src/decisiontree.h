/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef DECISIONTREE_H
#define DECISIONTREE_H


/**
 * <p>
 * This class defined custom decision tree types.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */


#include "common.h"
#include "types.h"
#include "rng.h"


class DecisionTree {
    public:
    VC<TreeNode> nodes;
    VD importances;

    private:
    template <class T> INLINE T customLoss(T x) {
        return sqrt(abs(x));
    }

private:
    friend class cereal::access;

    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( nodes, importances);
      }


    public:

    DecisionTree() { }

    template <class T> DecisionTree(VC<VC<T>> &features, VC<T> &results, RandomForestConfig &config, int seed) {
        RNG r(seed);

        if (config.computeImportances) {
            importances = VD(features[0].S);
        }

        VI chosenGroups(features.S);
        if (config.groupFeature == -1 && config.groups.S == 0) {
            REP(i, (int)(features.S * config.bagSize)) chosenGroups[r.next(features.S)]++;
        } else if (config.groupFeature != -1) {
            assert(config.groupFeature >= 0 && config.groupFeature < features.S);
            unordered_map<T, int> groups;
            int groupsNo = 0;
            REP(i, features.S) if (!groups.count(features[i][config.groupFeature])) {
                groups[features[i][config.groupFeature]] = groupsNo++;
            }
            VI groupSize(groupsNo);
            REP(i, (int)(groupsNo * config.bagSize)) groupSize[r.next(groupsNo)]++;
            REP(i, features.S) chosenGroups[i] = groupSize[groups[features[i][config.groupFeature]]];
        } else {
            assert(config.groups.S == features.S);
            int groupsNo = 0;
            for (int x : config.groups) groupsNo = max(groupsNo, x + 1);
            VI groupSize(groupsNo);
            REP(i, (int)(groupsNo * config.bagSize)) groupSize[r.next(groupsNo)]++;
            REP(i, features.S) chosenGroups[i] = groupSize[config.groups[i]];
        }

        int bagSize = 0;
        REP(i, features.S) if (chosenGroups[i]) bagSize++;

        VI bag(bagSize);
        VI weight(features.S);

        int pos = 0;

        REP(i, features.S) {
            weight[i] = config.useBootstrapping ? chosenGroups[i] : min(1, chosenGroups[i]);
            if (chosenGroups[i]) bag[pos++] = i;
        }

        TreeNode root;
        root.level = 0;
        root.left = 0;
        root.right = pos;
        nodes.PB(root);

        VI stack;
        stack.PB(0);

        while (stack.S) {
            bool equal = true;

            int curNode = stack[stack.S - 1];
            stack.pop_back();

            int bLeft = nodes[curNode].left;
            int bRight = nodes[curNode].right;
            int bSize = bRight - bLeft;

            int totalWeight = 0;
            T totalSum = 0;
            FOR(i, bLeft, bRight) {
                totalSum += results[bag[i]] * weight[bag[i]];
                totalWeight += weight[bag[i]];
            }

            assert(bSize > 0);

            FOR(i, bLeft + 1, bRight) if (results[bag[i]] != results[bag[i - 1]]) {
                equal = false;
                break;
            }

            if (equal || bSize <= config.maxNodeSize || nodes[curNode].level >= config.maxLevel) {
                nodes[curNode].result = totalSum / totalWeight;
                continue;
            }

            int bestFeature = -1;
            int bestLeft = 0;
            int bestRight = 0;
            T bestValue = 0;
            T bestLoss = 1e99;

            const int randomFeatures = config.randomFeatures[min(nodes[curNode].level, (int)config.randomPositions.S - 1)];
            REP(i, randomFeatures) {

                int featureID = config.featuresUsed.S ? config.featuresUsed[r.next(config.featuresUsed.S)] : config.featuresIgnored + r.next(features[0].S - config.featuresIgnored);

                T vlo, vhi;
                vlo = vhi = features[bag[bLeft]][featureID];
                FOR(j, bLeft + 1, bRight) {
                    vlo = min(vlo, features[bag[j]][featureID]);
                    vhi = max(vhi, features[bag[j]][featureID]);
                }
                if (vlo == vhi) continue;

                const int randomPositions = config.randomPositions[min(nodes[curNode].level, (int)config.randomPositions.S - 1)];
                REP(j, randomPositions) {
                    T splitValue = features[bag[bLeft + r.next(bSize)]][featureID];
                    if (splitValue == vlo) {
                        j--;
                        continue;
                    }

                    T sumLeft = 0;
                    int totalLeft = 0;
                    int weightLeft = 0;
                    FOR(k, bLeft, bRight) {
                        int p = bag[k];
                        if (features[p][featureID] < splitValue) {
                            sumLeft += results[p] * weight[p];
                            weightLeft += weight[p];
                            totalLeft++;
                        }
                    }

                    T sumRight = totalSum - sumLeft;
                    int weightRight = totalWeight - weightLeft;
                    int totalRight = bSize - totalLeft;

                    if (totalLeft == 0 || totalRight == 0)
                        continue;

                    T meanLeft = sumLeft / weightLeft;
                    T meanRight = sumRight / weightRight;
                    T loss = 0;

                    loss = (1 - meanLeft) * (1 - meanLeft) * sumLeft +
                        meanLeft * meanLeft * (weightLeft - sumLeft) +
                        (1 - meanRight) * (1 - meanRight) * sumRight +
                        meanRight * meanRight * (weightRight - sumRight);

                    if (config.lossFunction == RandomForestConfig::MSE) {
                        FOR(k, bLeft, bRight) {
                            int p = bag[k];
                            if (features[p][featureID] < splitValue) {
                                loss += (results[p] - meanLeft)  * (results[p] - meanLeft)  * weight[p];
                            } else {
                                loss += (results[p] - meanRight) * (results[p] - meanRight) * weight[p];
                            }
                            if (loss > bestLoss) break; //OPTIONAL
                        }
                    } else if (config.lossFunction == RandomForestConfig::MCE) {
                        FOR(k, bLeft, bRight) {
                            int p = bag[k];
                            if (features[p][featureID] < splitValue) {
                                loss += abs(results[p] - meanLeft)  * (results[p] - meanLeft)  * (results[p] - meanLeft)  * weight[p];
                            } else {
                                loss += abs(results[p] - meanRight) * (results[p] - meanRight) * (results[p] - meanRight) * weight[p];
                            }
                            if (loss > bestLoss) break; //OPTIONAL
                        }
                    } else if (config.lossFunction == RandomForestConfig::MAE) {
                        FOR(k, bLeft, bRight) {
                            int p = bag[k];
                            if (features[p][featureID] < splitValue) {
                                loss += abs(results[p] - meanLeft)  * weight[p];
                            } else {
                                loss += abs(results[p] - meanRight) * weight[p];
                            }
                            if (loss > bestLoss) break; //OPTIONAL
                        }
                    } else if (config.lossFunction == RandomForestConfig::CUSTOM) {
                        FOR(k, bLeft, bRight) {
                            int p = bag[k];
                            if (features[p][featureID] < splitValue) {
                                loss += customLoss(results[p] - meanLeft)  * weight[p];
                            } else {
                                loss += customLoss(results[p] - meanRight) * weight[p];
                            }
                            if (loss > bestLoss) break; //OPTIONAL
                        }
                    }

                    if (loss < bestLoss) {
                        bestLoss = loss;
                        bestValue = splitValue;
                        bestFeature = featureID;
                        bestLeft = totalLeft;
                        bestRight = totalRight;
                        if (loss == 0) goto outer;
                    }
                }
            }
            outer:

            if (bestLeft == 0 || bestRight == 0) {
                nodes[curNode].result = totalSum / totalWeight;
                continue;
            }

            if (config.computeImportances) {
                importances[bestFeature] += bRight - bLeft;
            }

            T mean = totalSum / totalWeight;

            T nextValue = -1e99;
            FOR(i, bLeft, bRight) if (features[bag[i]][bestFeature] < bestValue) nextValue = max(nextValue, features[bag[i]][bestFeature]);

            TreeNode left;
            TreeNode right;

            left.level = right.level = nodes[curNode].level + 1;
            nodes[curNode].feature = bestFeature;
            nodes[curNode].value = (bestValue + nextValue) / 2.0;
            if (!(nodes[curNode].value > nextValue)) nodes[curNode].value = bestValue;
            nodes[curNode].left = nodes.S;
            nodes[curNode].right = nodes.S + 1;

            int bMiddle = bRight;
            FOR(i, bLeft, bMiddle) {
                if (features[bag[i]][nodes[curNode].feature] >= nodes[curNode].value) {
                    swap(bag[i], bag[--bMiddle]);
                    i--;
                    continue;
                }
            }

            assert(bestLeft == bMiddle - bLeft);
            assert(bestRight == bRight - bMiddle);

            left.left = bLeft;
            left.right = bMiddle;
            right.left = bMiddle;
            right.right = bRight;

            stack.PB(nodes.S);
            stack.PB(nodes.S + 1);

            nodes.PB(left);
            nodes.PB(right);

        }

        nodes.shrink_to_fit();
    }

    template < class T > double predict(VC < T > &features) {
        TreeNode *pNode = &nodes[0];
        while (true) {
            if (pNode->feature < 0) return pNode->result;
            pNode = &nodes[features[pNode->feature] < pNode->value ? pNode->left : pNode->right];
        }
    }

};

#endif // DECISIONTREE_H

