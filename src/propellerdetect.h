#ifndef PROPELLERDETECTOR_H
#define PROPELLERDETECTOR_H

/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include <string>
#include "pdsimageparameters.h"
#include "indextabparser.h"
#include "types.h"

#include <iostream>
#include <vector>
#include <mutex>

/**
 * <p>
 * This class expose routines used to detect propeller.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;

class PropellerDetect
{

private:
    static const double POSITION_TOLERANCE;

        //Ground truth token positions
        static const int TOKEN_IMAGE ;
        static const int TOKEN_LINE;
        static const int TOKEN_SAMPLE;
        static const int TOKEN_NICKNAME;
public:
    vector<PdsImageParameters> readLabelFile(string labelFilePath, Options opt);
    vector<string> loadGroundTruth(string fileName);
    void train(string trainLabel, string groundTruth, string save_model_to, Options opt);
    void test(string testLabel, string groundTruth, string model_from, string save_candidates_to, Options opt);
    double score_and_link(string candidates_from, Options opt);
    vector<pair<int, vector<string> > > compute_candidates(vector<string> answer, vector<string> groundTruth,  int& numDetected, double& positionScore, Options opt);
    PropellerDetect();
    ~PropellerDetect();
};

#endif // PROPELLERDETECTOR_H
