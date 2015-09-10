/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */


#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H

/**
 * <p>
 * This file contained global variables used for predicting.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

#include "common.h"
#include "rng.h"


static RNG rng;
RNG gRNG(1);
const double PI = 2 * acos(0); //constexp

//int gY=0,gX=0;
//string gImageId;

//double startTimeProgram;


//double timePassed = 0;


const int RADIUS_BAGS = 2048;

//double minRadius = 1e9;
//double maxRadius = 0;

//double allSum = 0;
//double allSum2 = 0;
//double allVar = 0;
//int    allTot = 0;

/*double bagSum[RADIUS_BAGS] = {0};
double bagSum2[RADIUS_BAGS] = {0};
double bagVar[RADIUS_BAGS] = {0};
double bagSumPos[RADIUS_BAGS] = {0};
double bagSumNeg[RADIUS_BAGS] = {0};
int bagSumTot[RADIUS_BAGS] = {0};
int bagSumPosTot[RADIUS_BAGS] = {0};
int bagSumNegTot[RADIUS_BAGS] = {0};*/


int currentImageId;

#endif // GLOBALVARIABLE_H

