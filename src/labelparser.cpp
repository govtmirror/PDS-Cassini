/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */


#include "labelparser.h"
#include "helper.h"
#include <regex>
#include "easyloggingcpp/easylogging++.h"
#include "detectorexceptions.h"


/**
 * <p>
 * This class will parse label files
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */



const string LabelParser::COMMENT_PATTERN = "/\\*.*\\*/";
const string LabelParser::DEFAULT_LINE_SEPARATOR = "\n";
const string LabelParser::OBJECT_PATTERN_TEMPLATE = "OBJECT\\s*=\\s*<ObjectType>[^\\w]+(.*?)END_OBJECT\\s*=\\s*<ObjectType>";
const  int LabelParser::OBJECT_CONTENT_INDEX = 1;
const string LabelParser::OBJECT_PROPERTY_PATTEN = "(\\w+)\\s*=\\s*((\\w+)|(\"[^\"]*\"))";
const string LabelParser::ROOT_PROPERTY_PATTEN = "<PropertyName>\\s*=\\s*((\\w+)|(\"[^\"]*\"))";
const  int LabelParser::OBJECT_PROPERTY_KEY_INDEX = 1;
const int LabelParser::OBJECT_PROPERTY_VALUE_INDEX = 2;
const int LabelParser::ROOT_PROPERTY_VALUE_INDEX = 1;

LabelParser::LabelParser(string labelFilePath)
{
    std::ifstream t(labelFilePath);
    if(!t.is_open() || !t.good()){
        throw LabelParseException("Access Error when opening label file " + labelFilePath);
    }
    t.seekg(0, std::ios::end);
    size_t size = t.tellg();
    std::string buffer(size, ' ');
    t.seekg(0);
    t.read(&buffer[0], size);
    originalLabelContent = buffer;
    labelContent = removeCommentString(originalLabelContent);

    labels = Helper::readTextFile(labelFilePath);
    t.close();

}


string LabelParser::getLabelContent() const
{
    return labelContent;
}

void LabelParser::setLabelContent(const string &value)
{
    labelContent = value;
}

string LabelParser::getOriginalLabelContent() const
{
    return originalLabelContent;
}

void LabelParser::setOriginalLabelContent(const string &value)
{
    originalLabelContent = value;
}
string LabelParser::removeCommentString(string content){

    std::regex comments_re(COMMENT_PATTERN);
    return std::regex_replace(content, comments_re, "");

}

vector<unordered_map<string, string> > LabelParser::parseObject(string objectType){

    vector<unordered_map<string, string> > ret;
    int currentIndex = 0;

    bool active = false;

    unordered_map<string, string> obj;

    for(int i=0; i<(int)labels.size(); ++i){

        vector<string> columns = Helper::split(labels[i], '=');

        transform(columns.begin(), columns.end(), columns.begin(), Helper::trim);

        if(columns.size() == 2){

            if(columns[0] == "OBJECT" && columns[1] == objectType){

                active = true;

            } else if(columns[0] == "END_OBJECT" && columns[1] == objectType    ){
                active = false;
                currentIndex++;
                ret.push_back(obj);
                obj = unordered_map<string, string>();
            }

            if(active){

                obj.insert(make_pair(columns[0], columns[1]));
            }
        }

    }

    return ret;

}

string LabelParser::parseRootProperty(string propertyName){

    std::regex objectPattern("<PropertyName>", std::regex::extended);
    string reg_exp = Helper::replace(ROOT_PROPERTY_PATTEN, "<PropertyName>", propertyName);
    objectPattern = std::regex(reg_exp);
    std::smatch matcher;
    if(std::regex_search (labelContent, matcher, objectPattern)){

        if(matcher.size() > ROOT_PROPERTY_VALUE_INDEX){
            return Helper::trimQuote(matcher[ROOT_PROPERTY_VALUE_INDEX]);
        }
    }

    return "";
}



LabelParser::~LabelParser()
{

}

