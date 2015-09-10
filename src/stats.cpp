/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "propellerdetect.h"
#include "types.h"
#include <string>
#include <iostream>
#include <fstream>
#include "tclap/CmdLine.h"
#include "easyloggingcpp/easylogging++.h"
#include "commonarg.h"
INITIALIZE_EASYLOGGINGPP

/**
 * <p>
 * This file defined the program to compute statistics from saved model.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;
using namespace TCLAP;

int main(int argc, char** argv){

    PropellerDetect detect;
    // Wrap everything in a try block.  Do this every time,
    // because exceptions will be thrown for problems.
    try {

        // Define the command line object.
        CmdLine cmd("Propeller Detector Testing. Load trained Random Forest and output predicted candidate propellers from given input data", ' ', "1.0");

        // Define a value argument and add it to the command line.

        ValueArg<string> save_candidates_arg("o","candidate","Path where candidate propellers were saved",false,"candidates.dat","string");
        cmd.add( save_candidates_arg);

        cmd.add( log_file_arg);


        // Define a switch and add it to the command line.
        cmd.add(  verbosity_arg );

        // Parse the args.
        cmd.parse( argc, argv );

        Options opt;
        opt.log_file = log_file_arg.getValue();

        detect.score_and_link(save_candidates_arg.getValue(), opt);

    } catch (ArgException &e)  // catch any exceptions
    { LOG(ERROR) << "error: " << e.error() << " for arg " << e.argId() << endl; }


    return 0;


}

