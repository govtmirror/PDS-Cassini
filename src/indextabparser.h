/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef INDEXTABPARSER_H
#define INDEXTABPARSER_H

#include <string>
#include <unordered_map>
#include <iostream>

#include "pdsimageparameters.h"

/**
 * <p>
 * This class will parse tab files.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;




class IndexTabParser
{
private:
    static const string CONFIG_LINE_PATTERN;
    static const string CONFIG_ELEMENT_PATTERN;
     unordered_map<string, int> columnIndex;
     int expectedColumnsCount;
     string imageFilePathBase;
    string tabFilePath;
public:
    IndexTabParser(string labelFilePath);
    IndexTabParser();
    PdsImageParameters parseTabLine(string configLine);


    ~IndexTabParser();
    string getTabFilePath() const;
    void setTabFilePath(const string &value);
    string getImageFilePathBase() const;
    void setImageFilePathBase(const string &value);
    int getExpectedColumnsCount() const;
    void setExpectedColumnsCount(int value);
};

#endif // INDEXTABPARSER_H
