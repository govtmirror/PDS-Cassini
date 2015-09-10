/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef TYPES_H
#define TYPES_H

#include <string>
#include "cereal/archives/binary.hpp"
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/utility.hpp>

/**
 * <p>
 * This class defined custom types.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;

enum PdsImagePixelType {
      PC_REAL,
      SUN_INTEGER,
      UNKNOWN_TYPE
};

typedef struct Options
{
    string train_label;
    string ground_truth;
    string model_file;
    string log_file;
    string log_file_cont;
    int start_index;
    int end_index;
    int time_limit;
    int max_trees;
    int num_threads;
    int max_candidates;
} Options;



#endif // TYPES_H

