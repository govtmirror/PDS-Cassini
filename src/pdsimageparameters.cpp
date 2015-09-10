/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "pdsimageparameters.h"
#include "detectorexceptions.h"

/**
 * <p>
 * This class contains pds image parameters entity.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */


string PdsImageParameters::getImageFilePath() const
{
    return imageFilePath;
}

void PdsImageParameters::setImageFilePath(const string &value)
{
    imageFilePath = value;
}

string PdsImageParameters::getInstrumentId() const
{
    return instrumentId;
}

void PdsImageParameters::setInstrumentId(const string &value)
{
    instrumentId = value;
}

string PdsImageParameters::getInstrumentModeId() const
{
    return instrumentModeId;
}

void PdsImageParameters::setInstrumentModeId(const string &value)
{
    instrumentModeId = value;
}

double PdsImageParameters::getRightAscension() const
{
    return rightAscension;
}

void PdsImageParameters::setRightAscension(double value)
{
    rightAscension = value;
}

void PdsImageParameters::setRightAscension(string value)
{
    rightAscension = stod(value);
}

double PdsImageParameters::getDeclination() const
{
    return declination;
}

void PdsImageParameters::setDeclination(double value)
{
    declination = value;
}
void PdsImageParameters::setDeclination(string value)
{
    try{
        declination = stod(value);
    } catch(...){
        throw DetectorException("Invalid double value for the declination");
    }


}

double PdsImageParameters::getTwistAngle() const
{
    return twistAngle;
}

void PdsImageParameters::setTwistAngle(double value)
{
    twistAngle = value;
}

void PdsImageParameters::setTwistAngle(string value)
{
    try{
        twistAngle = stod(value);
    } catch(...){
        throw DetectorException("Invalid double value for the twist angle");
    }

}


string PdsImageParameters::getStartTime() const
{
    return startTime;
}

void PdsImageParameters::setStartTime(const string &value)
{
    startTime = value;
}

void PdsImageParameters::setScPlanetPositionVector(double x, double y, double z){
    scPlanetPositionVector = vector<double>(3);
    scPlanetPositionVector[0] = x;
    scPlanetPositionVector[1] = y;
    scPlanetPositionVector[2] = z;

}

void PdsImageParameters::setScPlanetPositionVector(string x, string y, string z){
    scPlanetPositionVector = vector<double>(3);

    try{
        scPlanetPositionVector[0] = stod(x);
        scPlanetPositionVector[1] = stod(y);
        scPlanetPositionVector[2] = stod(z);
    } catch(...){
        throw DetectorException("Invalid double values for the planet position vector");
    }


}

vector<double> PdsImageParameters::getScPlanetPositionVector() const{
    return scPlanetPositionVector;

}

PdsImageParameters::PdsImageParameters()
{

}

PdsImageParameters::~PdsImageParameters()
{

}

