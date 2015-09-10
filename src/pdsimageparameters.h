/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef PDSIMAGEPARAMETERS_H
#define PDSIMAGEPARAMETERS_H

#include <string>
#include <iostream>
#include <vector>

/**
 * <p>
 * This class contains pds image parameters entity.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;


class PdsImageParameters
{
public:

    string imageFilePath;
    string instrumentId;
     string instrumentModeId;
     double rightAscension;
     double declination;
     double twistAngle;
     vector<double> scPlanetPositionVector;
       string startTime;
    PdsImageParameters();
    ~PdsImageParameters();
    string getImageFilePath() const;
    void setImageFilePath(const string &value);
    string getInstrumentId() const;
    void setInstrumentId(const string &value);
    string getInstrumentModeId() const;
    void setInstrumentModeId(const string &value);
    double getRightAscension() const;
    void setRightAscension(double value);
    void setRightAscension(string value);
    double getDeclination() const;
    void setDeclination(double value);
    void setDeclination(string value);
    double getTwistAngle() const;
    void setTwistAngle(double value);
    void setTwistAngle(string value);
    string getStartTime() const;
    void setStartTime(const string &value);
    void setScPlanetPositionVector(double x, double y, double z);
    void setScPlanetPositionVector(string x, string y, string z);
    vector<double> getScPlanetPositionVector() const;
};

#endif // PDSIMAGEPARAMETERS_H
