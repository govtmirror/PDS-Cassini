/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "indextabparser.h"

#include "labelparser.h"

#include "helper.h"

#include "CSV/CSV.hpp"

#include <regex>
#include "easyloggingcpp/easylogging++.h"

/**
 * <p>
 * This class will parse tab files.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */


const string IndexTabParser::CONFIG_LINE_PATTERN = "(((\"[^\"]+\")|([^,]+)),)+";
const string IndexTabParser::CONFIG_ELEMENT_PATTERN = "((\"[^\"]+\")|([^,]+)),";



string IndexTabParser::getTabFilePath() const
{
    return tabFilePath;
}

void IndexTabParser::setTabFilePath(const string &value)
{
    tabFilePath = value;
}

string IndexTabParser::getImageFilePathBase() const
{
    return imageFilePathBase;
}

void IndexTabParser::setImageFilePathBase(const string &value)
{
    imageFilePathBase = value;
}

int IndexTabParser::getExpectedColumnsCount() const
{
    return expectedColumnsCount;
}

void IndexTabParser::setExpectedColumnsCount(int value)
{
    expectedColumnsCount = value;
}
IndexTabParser::IndexTabParser(){

}

IndexTabParser::IndexTabParser(string labelFilePath)
{

    // Constructor of LabelParser will check that labelFilePath is null nor empty.
    LabelParser parser(labelFilePath);
    // We have checked that the labelFilePath is legal
    tabFilePath = parser.parseRootProperty("IMAGE_INDEX_TABLE");
    //imageFilePathBase = "";
    int last_sep = labelFilePath.find_last_of("/\\");
    if(last_sep == string::npos)
		imageFilePathBase = "";
	else
		imageFilePathBase = labelFilePath.substr(0, last_sep + 1);

    tabFilePath = Helper::replace(labelFilePath, ".lbl", ".tab");


    //tabFilePath = imageFilePathBase + tabFilePath;
    if(tabFilePath.empty()){
        throw DetectorException("The index tab file path was not found in label file " + labelFilePath);
    }

    vector<unordered_map<string, string> > columnObjects = parser.parseObject("COLUMN");
    if (columnObjects.size() == 0) {
        LOG(ERROR) << "No column object found in the label file" << labelFilePath<< endl;
        throw DetectorException("No column object found in the label file " + labelFilePath);
    }
    int currentIndex = 0;

    try{

        for(auto && column: columnObjects){
            columnIndex.insert(make_pair(column.at("NAME"), currentIndex));

            auto numItems = column.find("ITEMS");

            if(numItems != column.end()){
                currentIndex += std::stoi(numItems->second);

            } else {

                ++currentIndex;
            }
        }

    } catch (const std::out_of_range& ) {

        LOG(ERROR) << "NAME entry not found in label file " + labelFilePath << endl;
        throw DetectorException("NAME entry not found in label file " + labelFilePath);
    }


    expectedColumnsCount = currentIndex;

    vector<string> columnsNames = {"FILE_SPECIFICATION_NAME", "INSTRUMENT_ID",
                                  "INSTRUMENT_MODE_ID", "RIGHT_ASCENSION",
                                  "DECLINATION", "TWIST_ANGLE", "SC_PLANET_POSITION_VECTOR",
                                  "START_TIME"};

    for(auto name:columnsNames){
        try{
            columnIndex.at(name);

        } catch (const std::out_of_range& ) {

            LOG(ERROR) << name + " column not found in label file " + labelFilePath << endl;

            throw DetectorException(name + " column not found in label file " + labelFilePath);
        }

    }




}

PdsImageParameters IndexTabParser::parseTabLine(string configLine){
	//LOG(ERROR) << configLine << endl;

    PdsImageParameters parameters;
    vector<string> columns;

    istringstream ss(configLine);

    jay::util::CSVread csv_read( &ss,
                                 jay::util::CSVread::strict_mode
                                 | jay::util::CSVread::process_empty_records
                                 );

    while( csv_read.ReadRecord() )
    {
        for( unsigned i = 0; i < csv_read.fields.size(); ++i )
        {
            columns.push_back(csv_read.fields[ i ]);
        }
    }

    transform(columns.begin(), columns.end(), columns.begin(), Helper::trim);

    // Extract image parameters

    try{

        parameters.setImageFilePath(imageFilePathBase
                                    + columns.at(columnIndex.at("FILE_SPECIFICATION_NAME")));

        parameters.setInstrumentId(columns.at(columnIndex.at("INSTRUMENT_ID")));

        parameters.setInstrumentModeId(columns.at(columnIndex.at("INSTRUMENT_MODE_ID")));

        parameters.setRightAscension(columns.at(columnIndex.at("RIGHT_ASCENSION")));

        parameters.setDeclination(columns.at(columnIndex.at("DECLINATION")));

        parameters.setTwistAngle(columns.at(columnIndex.at("TWIST_ANGLE")));

        int scPlanetPositionVectorIndex = columnIndex.at("SC_PLANET_POSITION_VECTOR");

        parameters.setScPlanetPositionVector(columns.at(scPlanetPositionVectorIndex),
                                             columns.at(scPlanetPositionVectorIndex + 1),
                                             columns.at(scPlanetPositionVectorIndex + 2));

        parameters.setStartTime(columns.at(columnIndex.at("START_TIME")));

    } catch (const std::out_of_range& ) {

        LOG(ERROR) << "Image parameters not found in the label file" << endl;
        throw DetectorException("Image parameters not found in the label file");

    }

    return parameters;

}

IndexTabParser::~IndexTabParser()
{

}

