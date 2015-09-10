/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "pdsimagereader.h"
#include "helper.h"
#include "labelparser.h"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <iterator>
#include <bitset>
#include "easyloggingcpp/easylogging++.h"

/**
 * <p>
 * This class will read a pds image.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */


const string PdsImageReader::IMG_FILE_SUFFIX = ".IMG";
const string PdsImageReader::LABEL_FILE_SUFFIX = ".LBL";
const string PdsImageReader::LABEL_OBJECT_TYPE_IMAGE = "IMAGE";
const string PdsImageReader::LABEL_OBJECT_KEY_LINES = "LINES";
const string PdsImageReader::LABEL_OBJECT_KEY_LINE_SAMPLES = "LINE_SAMPLES";
const string PdsImageReader::LABEL_OBJECT_KEY_SAMPLE_BITS = "SAMPLE_BITS";
const string PdsImageReader::LABEL_OBJECT_KEY_SAMPLE_TYPE = "SAMPLE_TYPE";
const string PdsImageReader::LABEL_OBJECT_KEY_LINE_PREFIX_BYTES = "LINE_PREFIX_BYTES";
const int PdsImageReader::MAXIMUM_GRAY_VALUE = 256;

PdsImageReader::PdsImageReader(string filePath)
{

    // Check file name pattern
    if(!Helper::endsWidth(Helper::toUpperCase(filePath), IMG_FILE_SUFFIX)){
        LOG(ERROR) << "The image file name " + filePath + " must have suffix " << endl;
        throw new DetectorException("The image file name " + filePath + " must have suffix ");
    }

    // Parse the label file.
    string labelFileName = Helper::trimCharacters(filePath, "", IMG_FILE_SUFFIX)
            + LABEL_FILE_SUFFIX;

    loadLabelParameters(labelFileName);

    // Read image data
    imageData = readImage(filePath);

}


vector<vector<double> > PdsImageReader::getImageData() const
{
    return imageData;
}

void PdsImageReader::setImageData(const vector<vector<double> > &value)
{
    imageData = value;
}
void PdsImageReader::loadLabelParameters(string labelFilePath){
    LabelParser parser(labelFilePath);
    originalLabelContent = parser.getOriginalLabelContent();
    vector<unordered_map<string, string> > imageObjects = parser.parseObject(LABEL_OBJECT_TYPE_IMAGE);
    if (imageObjects.size() < 1) {
        LOG(ERROR) << "No image object found in the label file " + labelFilePath << endl;
        throw DetectorException("No image object found in the label file " + labelFilePath);
    }

    try{

        unordered_map<string, string> lastImage = imageObjects.at(imageObjects.size() - 1);
        imageHeight = stoi(lastImage.at(LABEL_OBJECT_KEY_LINES));
        imageWidth = stoi(lastImage.at(LABEL_OBJECT_KEY_LINE_SAMPLES));
        pixelType = Helper::toPdsImagePixelType(lastImage.at(LABEL_OBJECT_KEY_SAMPLE_TYPE));
        elementSize = stoi(lastImage.at(LABEL_OBJECT_KEY_SAMPLE_BITS))/8;

        auto it = lastImage.find(LABEL_OBJECT_KEY_LINE_PREFIX_BYTES);

        linePrefixLength = 0;

        if(it != lastImage.end()){
            linePrefixLength = stoi(it->second);
        }

    } catch (const std::out_of_range& ) {

        LOG(ERROR) << "Error while loading label parameters in label file " + labelFilePath << endl;
        throw DetectorException("Error while loading label parameters in label file " + labelFilePath);
    } catch (const std::invalid_argument& ) {

        LOG(ERROR) << "Error while loading label parameters in label file " + labelFilePath << endl;
        throw DetectorException("Error while loading label parameters in label file " + labelFilePath);
    }
}

vector<vector<double> > PdsImageReader::readImage(string fileName){

    int bufferLen = elementSize * imageWidth;
    int rawDataLen = (linePrefixLength + bufferLen) * imageHeight;
    vector<vector<double> > imageDataToRead;

    vector<char> buffer = Helper::ReadAllBytes(fileName);
    int available = Helper::getBinaryFileSize(fileName);

    if((int)buffer.size() < rawDataLen){
        LOG(ERROR) << "The IMG file " + fileName + " is malformed. Header-skip error." << endl;
        throw DetectorException("The IMG file " + fileName + " is malformed. Header-skip error.");
    }

    ifstream input( fileName, std::ios::binary );
    if(!input.is_open() || !input.good()){
        LOG(ERROR) << "Access Error when opening file " + fileName << endl;
        throw DetectorException("Access Error when opening file " + fileName);
    }
    char c;
    for(int i=0; i< available - rawDataLen;++i){
        input.read(&c, sizeof(char));
        imageHeader.push_back(c);
    }
    float f;

    auto it = buffer.begin();
    advance(it, buffer.size()- rawDataLen);

    int pos = buffer.size() - rawDataLen;
    imageDataToRead = vector<vector<double> >(imageHeight, vector<double>(imageWidth, 0));
    int numRows = 0;

    while(pos < available){
        // skip linePrefixLenght by reading from pos+linePrefixLength
        // We should be able to read bufferLen
        if(available- pos - linePrefixLength < bufferLen){
            // Error
            LOG(ERROR) << "The IMG file  " + fileName + " is malformed. Image line was not fully read" << endl;
            throw DetectorException("The IMG file  " + fileName + " is malformed. Image line was not fully read");
        }

        pos += linePrefixLength;

        for(int i=0; i< linePrefixLength;++i){
            input.read(&c, sizeof(char));
        }

        for (int i = 0; i < imageWidth; i++){

            if (pixelType == SUN_INTEGER) {

                input.read(&c, sizeof(char));
                imageDataToRead[numRows][i] = (int(c) + MAXIMUM_GRAY_VALUE) % MAXIMUM_GRAY_VALUE;
                pos++;
            } else if (pixelType == PC_REAL) {
                input.read((char*)&f, sizeof(float));
                imageDataToRead[numRows][i] = f;
                pos +=4;
            } else {
                LOG(ERROR) << "The pixel types of this IMG file " + fileName + " is not supported" << endl;
                throw DetectorException("The pixel types of this IMG file " + fileName + " is not supported");

            }

        }

        ++numRows;

    }

    if (numRows != imageHeight) {
        LOG(ERROR) << "The IMG file " +  fileName + " is malformed. Number of image rows must be " + to_string(imageHeight) << endl;
        throw DetectorException("The IMG file " +  fileName + " is malformed. Number of image rows must be " + to_string(imageHeight));
    }
    return imageDataToRead;
}

PdsImageReader::~PdsImageReader()
{

}

