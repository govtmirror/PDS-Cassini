/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef PDSIMAGEREADER_H
#define PDSIMAGEREADER_H

#include <string>
#include <iostream>
#include "types.h"
#include <vector>

/**
 * <p>
 * This class will read a pds image.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;

class PdsImageReader
{
private:
    static const string IMG_FILE_SUFFIX ;
          static const string LABEL_FILE_SUFFIX ;
          static const string LABEL_OBJECT_TYPE_IMAGE ;
          static const string LABEL_OBJECT_KEY_LINES ;
          static const string LABEL_OBJECT_KEY_LINE_SAMPLES ;
          static const string LABEL_OBJECT_KEY_SAMPLE_BITS ;
          static const string LABEL_OBJECT_KEY_SAMPLE_TYPE ;
          static const string LABEL_OBJECT_KEY_LINE_PREFIX_BYTES;
          static const int MAXIMUM_GRAY_VALUE ;

           int imageHeight;
          int imageWidth;
          int linePrefixLength;
          int elementSize;
          PdsImagePixelType pixelType;
          vector<char> imageHeader;
          string originalLabelContent;
          vector<vector<double> >imageData;
          void loadLabelParameters(string labelFilePath) ;
          vector<vector<double> > readImage(string fileName);
public:
    PdsImageReader(string filePath);
    ~PdsImageReader();
    vector<vector<double> > getImageData() const;
    void setImageData(const vector<vector<double> > &value);
};

#endif // PDSIMAGEREADER_H
