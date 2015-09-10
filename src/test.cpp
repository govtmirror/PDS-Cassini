/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "propellerdetect.h"
#include "types.h"
#include "tclap/CmdLine.h"
#include <string>
#include <iostream>
#include <fstream>
#include "easyloggingcpp/easylogging++.h"
#include "commonarg.h"


/**
 * <p>
 * This file defined the program to Load trained Random Forest and output predicted candidate propellers.
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
        CmdLine cmd("Propeller Detector Testing. Load trained Random Forest and output predicted candidate propellers from given input data", ' ', "1.0");

        // Define a value argument and add it to the command line.
        ValueArg<string> test_label_arg("t","testlabel","Path to the test label",false,"quick_index.lbl","string");
        cmd.add( test_label_arg);

        cmd.add( ground_truth_arg);

        ValueArg<string> save_model_arg("m","model","Path where the random forest model has been saved",false,"training_model.dat","string");
        cmd.add( save_model_arg);

        ValueArg<string> save_candidates_arg("o","candidate","Path where to save candidate propellers",false,"candidates.dat","string");
        cmd.add( save_candidates_arg);

        cmd.add( log_file_arg);

        cmd.add( log_file_cont_arg);

        cmd.add( start_index_arg);

        cmd.add( end_index_arg);

        cmd.add( num_threads_arg);

        ValueArg<int> max_candidates_arg("N","maxcandidates","maximum number of most promising candidate propellers",false,10000,"int");
        cmd.add( max_candidates_arg);

        // Define a switch and add it to the command line.
        cmd.add(  verbosity_arg );

        // Parse the args.
        cmd.parse( argc, argv );

        Options opt;

        opt.end_index = end_index_arg.getValue();
        opt.start_index = start_index_arg.getValue();
        opt.num_threads = num_threads_arg.getValue();
        opt.log_file = log_file_arg.getValue();
        opt.log_file_cont = log_file_cont_arg.getValue();
        opt.max_candidates = max_candidates_arg.getValue();

        // Get the value parsed by each arg.

        detect.test(test_label_arg.getValue(),ground_truth_arg.getValue(), save_model_arg.getValue(), save_candidates_arg.getValue(), opt);

    } catch (ArgException &e)  // catch any exceptions
    { LOG(ERROR) << "error: " << e.error() << " for arg " << e.argId() << endl; }


    return 0;


}

