/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef COMMONARG_H
#define COMMONARG_H

#include "tclap/CmdLine.h"
#include <string>


using namespace std;
using namespace TCLAP;


/**
 * <p>
 * This file contains commons argument variables.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

// Define a value argument and add it to the command line.
ValueArg<string> train_label_arg("t","trainlabel","Path to the train label",false,"quick_index.lbl","string");

ValueArg<string> ground_truth_arg("g","groundtruth","Path to the ground truth",false,"full_ground_truth.csv","string");

ValueArg<string> save_model_arg("m","model","Path where to save the random forest model",false,"training_model.dat","string");

ValueArg<string> log_file_arg("","logfile","Path to where logs will be saved",false,"output_answer.txt","string");

ValueArg<string> log_file_cont_arg("","logfilecont","Path to where continious logs will be saved",false,"output_answer_continious.txt","string");

ValueArg<int> start_index_arg("s","start","Start index of images from .lbl to use",false,-1,"int");

ValueArg<int> end_index_arg("e","end","End index of images from .lbl to use",false,-1,"int");

ValueArg<int> time_limit_arg("l","limit","Time limit (or no limit) for training Random Forest in minutes",false,55,"int");

ValueArg<int> max_trees_arg("d","maxtrees","Maximum number of trees in Random Forest",false,400,"int");

ValueArg<int> num_threads_arg("c","numthreads","Number of threads",false,1,"int");

// Define a switch and add it to the command line.
SwitchArg verbosity_arg("v","verbose","Turn on verbosity", false);


#endif // COMMONARG_H
