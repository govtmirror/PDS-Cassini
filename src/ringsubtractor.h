/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef RINGSUBTRACTOR_H
#define RINGSUBTRACTOR_H


#include "simplematrix.h"
#include "common.h"
#include "types.h"
#include "easyloggingcpp/easylogging++.h"

/**
 * <p>
 * This class defined the ring subtractor type.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */



class RingSubtractor {

public:
    static VD calculateMinMax(VVD &array) {
        double mn = array[0][0];
        double mx = mn;
        for (VD &v : array) for (double d : v) {
            mn = min(mn, d);
            mx = max(mx, d);
        }
        return {mn, mx};
    }

    static void normalize(VVD &array) {
        VD valMinMax = calculateMinMax(array);
        double range = valMinMax[1] - valMinMax[0];
        double minimum = valMinMax[0];
        if (range > 0) {
            for (VD &v : array) for (double &d : v) d = (d - minimum) / range;
        }
    }

    static void elementaryScale(VVD &array, double scale) {
        for (VD &v : array) for (double &d : v) d *= scale;
    }

    static void elementaryAdd(VVD &array, double b) {
        for (VD &v : array) for (double &d : v) d += b;
    }

    VD findBestOffset(VVD &imageData, VVD &radius, VVD &longitude) {
        int imageHeight = imageData.S;
        int imageWidth = imageData[0].S;
        VVD backPlaneX = VVD(imageHeight, VD(imageWidth));
        VVD backPlaneY = VVD(imageHeight, VD(imageWidth));
        double longitudeInRadian = 0;

        for (int i = 0; i < imageHeight; i++) {
            for (int j = 0; j < imageWidth; j++) {
                longitudeInRadian = longitude[i][j] / 180.0 * PI;
                backPlaneX[i][j] = radius[i][j] * cos(longitudeInRadian);
                backPlaneY[i][j] = radius[i][j] * sin(longitudeInRadian);
            }
        }

        double searchWidth = 50;
        double searchLeft = -searchWidth;
        double searchRight = searchWidth;
        double searchTop = -searchWidth;
        double searchBottom = searchWidth;
        double searchStep = 1;
        VD minimumOffset(2);

        double minimumEnergy = 1e20;
        double offsetX = 0;
        double offsetY = 0;
        if (true) {
            double noTerminateBefore = 10;
            double terminateN = 40;
            double terminateGradient = 1e-8;
            double lastEnergy = -1;
            int numIteration = 0;
            offsetX = 0;
            offsetY = 0;
            double initialEnergy = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, 0, 0);
            while (offsetX <= searchRight && offsetX >= searchLeft && offsetY <= searchBottom && offsetY >= searchTop) {
                double energy = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, offsetX, offsetY);
                if (energy < minimumEnergy) {
                    minimumOffset[0] = max(searchLeft, min(searchRight, offsetX));
                    minimumOffset[1] = max(searchTop,  min(searchBottom, offsetY));
                    minimumEnergy = energy;
                }

                if ((lastEnergy >= 0 && abs(energy - lastEnergy) < searchStep * 0.001 && numIteration > noTerminateBefore) || numIteration > terminateN)
                    break;

                double energyRight = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, offsetX + 1, offsetY);
                double energyBelow = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, offsetX, offsetY + 1);
                double gradientX = energyRight - energy;
                double gradientY = energyBelow - energy;
                double gradientLen = sqrt(gradientX * gradientX + gradientY * gradientY);
                if (gradientLen < terminateGradient) break;

                offsetX -= searchStep * gradientX / gradientLen;
                offsetY -= searchStep * gradientY / gradientLen;

                ++numIteration;
                lastEnergy = energy;
            }
            double finalEnergy = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, offsetX, offsetY);
            //LOG(INFO)<< "Final Energy "<< finalEnergy << initialEnergy<< offsetX<< offsetY<<endl;
        } else {
            offsetX = searchLeft;
            while (offsetX <= searchRight) {
                offsetY = searchTop;
                while (offsetY <= searchBottom) {
                    double energy = calculateOffsetEnergy(imageData, backPlaneX, backPlaneY, offsetX, offsetY);
                    if (energy < minimumEnergy) {
                        minimumOffset[0] = offsetX;
                        minimumOffset[1] = offsetY;
                        minimumEnergy = energy;
                    }
                    offsetY += searchStep;
                }
                offsetX += searchStep;
            }
        }
        return minimumOffset;
    }

    double calculateOffsetEnergy(VVD &imageData, VVD &backPlaneX, VVD &backPlaneY, double offsetX, double offsetY) {
        int imageHeight = imageData.S;
        int imageWidth = imageData[0].S;
        double roiInf = 0.3;
        double roiSup = 1.0 - roiInf;
        int roiTop = (int)round(imageHeight * roiInf);
        int roiLeft = (int)round(imageWidth * roiInf);
        int roiBottom = (int)round(imageHeight * roiSup);
        int roiRight = (int)round(imageWidth * roiSup);
        int rangeX = roiRight - roiLeft;
        int rangeY = roiBottom - roiTop;

        // Prepare the radius in ROI area for given offset
        double valX, valY;
        VVD offsetRadius(rangeY, VD(rangeX, 0));
        for (int i = roiTop; i < roiBottom; i++) {
            for (int j = roiLeft; j < roiRight; j++) {
                valX = bilinearInterpolation(backPlaneX, j + offsetX, i + offsetY);
                valY = bilinearInterpolation(backPlaneY, j + offsetX, i + offsetY);
                offsetRadius[i-roiTop][j-roiLeft] = sqrt(valX * valX+valY * valY);
            }
        }

        // Calculate variance in each bin
        int numBins = (int)ceil(sqrt(rangeX * rangeX + rangeY * rangeY));
        VD valMinMax = calculateMinMax(offsetRadius);
        VD sum(numBins);
        VD sumSquare(numBins);
        VD hist(numBins);
        double radiusRange = valMinMax[1] - valMinMax[0];
        if (radiusRange <= 0) return -1.0;

        if (numBins <= 1) return -1.0;

        double binWidth = radiusRange / (numBins - 1);
        int idx = 0;
        double pixelVal = 0;
        for (int i = 0; i < rangeY; i++) {
            for (int j = 0; j < rangeX; j++) {
                idx = (int)round((offsetRadius[i][j] - valMinMax[0]) / binWidth);
                hist[idx] += 1.0;
                pixelVal = imageData[i + roiTop][j + roiLeft];
                sum[idx] += pixelVal;
                sumSquare[idx] += pixelVal * pixelVal;
            }
        }

        double energy = 0;
        for (int i = 0; i < numBins; i++) {
            if (hist[i] > 0) {
                energy += max(sumSquare[i] - sum[i] * sum[i] / hist[i], 0.0);
            }
        }

        return energy;
    }

    static double bilinearInterpolation(VVD &array, double x, double y) {
        int floorX = (int)floor(x);
        int floorY = (int)floor(y);
        double u = x - floorX;
        double v = y - floorY;
        return array[floorY][floorX] * (1 - v) * (1 - u) + array[floorY][floorX + 1] * (1 - v) * u +
               array[floorY + 1][floorX] * v * (1 - u) + array[floorY + 1][floorX + 1] * v * u;
    }

    static VVD subtract(VVD &imageData, Transformer &transformer, VD &optimizedOffset) {
        VVD radius = transformer.getRadiusArray(false, optimizedOffset[0], optimizedOffset[1]);

        VD radiusMinMax = calculateMinMax(radius);
        assert(radiusMinMax[0] < radiusMinMax[1]);

        VD binsAverage = calculateBinsAverage(radiusMinMax[0], radiusMinMax[1],	imageData, radius);
        return subtractByBins(radiusMinMax[0], radiusMinMax[1],	binsAverage, imageData, radius);
    }

    static VD calculateBinsAverage(double minimum, double maximum, VVD &imageData, VVD &radius) {
        int numBins = (int)ceil(sqrt(2.0) * imageData.S);
        VD average(numBins);
        VD hist(numBins);
        double radiusRange = maximum - minimum;
        double binWidth = radiusRange / (numBins - 1);
        int idx = 0;

        for (int i = 0; i < imageData.S; i++) {
            for (int j = 0; j < imageData[i].S; j++) {
                idx = (int)round((radius[i][j] - minimum) / binWidth);
                hist[idx] += 1.0;
                average[idx] += imageData[i][j];
            }
        }

        for (int i = 0; i < numBins; i++) {
            if (hist[i] > 0) {
                average[i] /= hist[i];
            }
        }
        return average;
    }

    static VVD subtractByBins(double minimum, double maximum, VD &binsAverage, VVD &imageData, VVD &radius) {
        VVD subtractedImage = imageData; // sure it works

        double radiusRange = maximum - minimum;
        int numBins = binsAverage.S;
        double binWidth = radiusRange / (numBins - 1);
        int idx = 0;

        for (int i = 0; i < imageData.S; i++) {
            for (int j = 0; j < imageData[i].S; j++) {
                idx = (int)round((radius[i][j] - minimum) / binWidth);
                subtractedImage[i][j] -= binsAverage[idx];
            }
        }

        return subtractedImage;
    }

public:
    VVD subtractRings(VVD &imageData, Transformer &transformer) {
        int ySize = imageData.S;
        int xSize = imageData[0].S;

        VVD originImageData(ySize, VD(xSize));

        for(int j=0; j < ySize; j++)
            for(int i=0; i < xSize; i++) {
                originImageData[j][i] = imageData[j][i];
            }

        VD grayMinMax = calculateMinMax(imageData);
        double grayRange = grayMinMax[1] - grayMinMax[0];
        if (grayRange <= 0) return VVD(0);

        normalize(imageData);

        int transformerImageSize = transformer.getImageSize();
        if (imageData.S <= 0 || imageData.S != transformerImageSize || imageData[0].S != transformerImageSize)
            return VVD(0);

        VVD radius = transformer.getRadiusArray(true);
        VVD longitude = transformer.getLongitudeArray(true);

        VD optimizedOffset = findBestOffset(imageData, radius, longitude);
        VVD subtractedImage = subtract(imageData, transformer, optimizedOffset);
        elementaryScale(subtractedImage, grayRange);

        return subtractedImage;
    }
};

#endif // RINGSUBTRACTOR_H

