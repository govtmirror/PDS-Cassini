/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "propellerdetect.h"
#include <string>
#include <iostream>
#include <fstream>
#include "tclap/CmdLine.h"
#include "types.h"
#include "easyloggingcpp/easylogging++.h"
#include "commonarg.h"


/**
 * <p>
 * This file defined the program to train on data and output trained Random Forest.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;
using namespace TCLAP;

INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv){

    PropellerDetect detect;
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        // Define the command line object.
        CmdLine cmd("Propeller Detector Training. Train on data and output trained Random Forest to a file.", ' ', "1.0");

        // Define a value argument and add it to the command line.
        cmd.add( train_label_arg);

        cmd.add( ground_truth_arg);

        cmd.add( save_model_arg);

        cmd.add( log_file_arg);
        cmd.add( start_index_arg);

        cmd.add( end_index_arg);

        cmd.add( time_limit_arg);

        cmd.add( max_trees_arg);
        cmd.add( num_threads_arg);

        // Define a switch and add it to the command line.
        cmd.add(  verbosity_arg );

        // Parse the args.
        cmd.parse( argc, argv );

        Options opt;

        opt.end_index = end_index_arg.getValue();
        opt.start_index = start_index_arg.getValue();
        opt.time_limit = time_limit_arg.getValue();
        opt.max_trees = max_trees_arg.getValue();
        opt.num_threads = num_threads_arg.getValue();
        opt.log_file = log_file_arg.getValue();

        // Get the value parsed by each arg.

        detect.train(train_label_arg.getValue(),ground_truth_arg.getValue(), save_model_arg.getValue(), opt);

    } catch (ArgException &e)  // catch any exceptions
    { LOG(ERROR) << "error: " << e.error() << " for arg " << e.argId() << endl; }

    return 0;


}
