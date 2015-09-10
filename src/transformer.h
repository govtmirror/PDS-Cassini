/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */


#ifndef TRANSFORMER_H
#define TRANSFORMER_H


/**
 * <p>
 * This class defined transformer type.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

#include "simplematrix.h"
#include "common.h"
#include "types.h"

struct TransformerData
{
	string instrumentId;
	string instrumentModeId;
	double rightAscension;
	double declination;
	double twistAngle;
	VD scPlanetPositionVector;

	TransformerData() { }

	TransformerData(string iId, string iModeId, double asc, double dec, double twi, VD& scPPV)
	{
		instrumentId = iId;
		instrumentModeId = iModeId;
		rightAscension = asc;
		declination = dec;
		twistAngle = twi;
		scPlanetPositionVector = scPPV;
	}
};

class Transformer {

private:
    static constexpr double SATURN_RIGHT_ASCENSION = 40.589;
    static constexpr double SATURN_DECLINATION = 83.537;
    static constexpr double FOV_ISSNA = 0.35;
    static constexpr double FOV_ISSWA = 3.48;
    static constexpr double IMAGE_SIZE_FULL = 1024;
    static constexpr double IMAGE_SIZE_SUM2 = 512;
    static constexpr double IMAGE_SIZE_SUM4 = 256;
    double fov;
    double N;
    double halfN;
    double delta;
    double rightAscension;
    double declination;
    double twistAngle;
    SimpleMatrix cMatrix;
    SimpleMatrix inverseCMatrix;
    SimpleMatrix rMatrix;
    SimpleMatrix inverseRMatrix;
    SimpleMatrix rInverseC;
    SimpleMatrix cInverseR;
    SimpleMatrix planetRing;
    VVD cachedRadiusArray;
    VVD cachedLongitudeArray;
    bool isCached;

public:

	Transformer(const TransformerData& dat) : Transformer(dat.instrumentId, dat.instrumentModeId, dat.rightAscension, dat.declination, dat.twistAngle, dat.scPlanetPositionVector) { }

    Transformer(string instrumentID, string instrumentModeID, double rightAscension, double declination, double twistAngle, const VD &scPlanetPositionVector) {
        assert(scPlanetPositionVector.S == 3);

        this->fov = parseInstrumentID(instrumentID);
        this->N = parseInstrumentModeID(instrumentModeID);
        this->rightAscension = rightAscension;
        this->declination = declination;
        this->twistAngle = twistAngle;
        initialize();
        SimpleMatrix tmp = SimpleMatrix({scPlanetPositionVector}).transpose();
        this->planetRing = rMatrix.mult(tmp);
        isCached = false;

        cachedRadiusArray.assign((int)(N + 0.5), VD((int)(N + 0.5)));
        cachedLongitudeArray.assign((int)(N + 0.5), VD((int)(N + 0.5)));
    }

    void pixelPosition2RadiusLongitude(double u, double v,double &LEN, double &DEG) {

        double a,b;
        a = (u - halfN + 0.5) * delta; b = (v - halfN + 0.5) * delta;
        double x,y,z;
        x = rInverseC.data[0] * a + rInverseC.data[1] * b + rInverseC.data[2];
        y = rInverseC.data[3] * a + rInverseC.data[4] * b + rInverseC.data[5];
        z = rInverseC.data[6] * a + rInverseC.data[7] * b + rInverseC.data[8];
        z = planetRing.data[2] / z;

        ASSERT(!((z * 0) != 0));
        ASSERT(!(z < 0));

        x *= z;                  y *= z;
        x -= planetRing.data[0]; y -= planetRing.data[1];

        LEN = sqrt(x * x + y * y);
        DEG = radian2Degree(atan2(y, x));
    }

    void clearCache() {
        isCached = false;
    }

    VVD getRadiusArray(bool useCache, double offsetX, double offsetY) {
        return getRadiusOrLongitudeArray(useCache, true, offsetX, offsetY);
    }

    VVD getLongitudeArray(bool useCache, double offsetX, double offsetY) {
        return getRadiusOrLongitudeArray(useCache, false, offsetX, offsetY);
    }

    VVD getRadiusArray(bool useCache) {
        return getRadiusOrLongitudeArray(useCache, true, 0, 0);
    }

    VVD getLongitudeArray(bool useCache) {
        return getRadiusOrLongitudeArray(useCache, false, 0, 0);
    }

private:
    static double parseInstrumentID(string instrumentID) {
        if (instrumentID == "ISSNA") {
            return FOV_ISSNA;
        } else if (instrumentID == "ISSWA") {
            return FOV_ISSWA;
        } else {
            assert(false);
        }
    }

    static double parseInstrumentModeID(string instrumentModeID) {
        if (instrumentModeID == "FULL") {
            return IMAGE_SIZE_FULL;
        } else if (instrumentModeID == "SUM2") {
            return IMAGE_SIZE_SUM2;
        } else if (instrumentModeID == "SUM4") {
            return IMAGE_SIZE_SUM4;
        } else {
            assert(false);
        }
    }

    void initialize() {
        halfN = 0.5 * N;
        delta = tan(degree2Radian(fov / 2.0)) / halfN;

        double r = degree2Radian(rightAscension);
        double d = degree2Radian(declination);
        double t = degree2Radian(twistAngle);
        double sinR = sin(r);
        double cosR = cos(r);
        double sinD = sin(d);
        double cosD = cos(d);
        double sinT = sin(t);
        double cosT = cos(t);
        cMatrix = SimpleMatrix({
                        {-sinR * cosT - cosR * sinD * sinT, cosR * cosT - sinR * sinD * sinT, cosD * sinT},
                        {sinR * sinT - cosR * sinD * cosT, -cosR * sinT - sinR * sinD * cosT, cosD * cosT},
                        {cosR * cosD, sinR * cosD, sinD}
                });
        inverseCMatrix = cMatrix.transpose();

        // Parameters for converting sky coordinate to ring coordinates
        double ra = degree2Radian(SATURN_RIGHT_ASCENSION);
        double dec = degree2Radian(SATURN_DECLINATION);
        double sinRA = sin(ra);
        double cosRA = cos(ra);
        double sinDec = sin(dec);
        double cosDec = cos(dec);
        rMatrix = SimpleMatrix({
                        {-sinRA, cosRA, 0},
                        {-cosRA * sinDec, -sinRA * sinDec, cosDec},
                        {cosRA * cosDec, sinRA * cosDec, sinDec}
                });
        inverseRMatrix = rMatrix.transpose();

        cInverseR = cMatrix.mult(inverseRMatrix);
        rInverseC = rMatrix.mult(inverseCMatrix);
    }

    static double degree2Radian(double d) {
        return d * PI / 180.0;
    }

    static double radian2Degree(double r) {
        return r * 180.0 / PI;
    }

    VVD getRadiusOrLongitudeArray(bool useCache, bool isRadius,	double offsetX, double offsetY) {
        // Check if cache is available
        if (useCache && isCached) {
            if (isRadius) {
                return cachedRadiusArray;
            } else if (!isRadius) {
                return cachedLongitudeArray;
            }
        } else {
            clearCache();
        }

        double LEN,DEG;
        for (int i = 0; i < N - 0.5; i++) {
            for (int j = 0; j < N - 0.5; j++) {
                pixelPosition2RadiusLongitude(j + offsetX, i + offsetY,LEN,DEG);
                cachedRadiusArray[i][j] = LEN;
                cachedLongitudeArray[i][j] = DEG;
            }
        }

        isCached = true;
        if (isRadius) {
            return cachedRadiusArray;
        } else {
            return cachedLongitudeArray;
        }
    }

public:
    int getImageSize() {
        return (int)round(N);
    }

};

#endif // TRANSFORMER_H
