/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef LABELPARSER_H
#define LABELPARSER_H

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>


/**
 * <p>
 * This class will parse label files.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;
class LabelParser
{

private:
    static const string COMMENT_PATTERN;
    static const string DEFAULT_LINE_SEPARATOR ;
    static const string OBJECT_PATTERN_TEMPLATE;
    static const  int OBJECT_CONTENT_INDEX;
    static const string OBJECT_PROPERTY_PATTEN;
    static const string ROOT_PROPERTY_PATTEN;
    static const  int OBJECT_PROPERTY_KEY_INDEX;
    static const int OBJECT_PROPERTY_VALUE_INDEX;
    static const int ROOT_PROPERTY_VALUE_INDEX;
    string labelContent;
    string originalLabelContent;

    vector<string> labels;

    string removeCommentString(string content);
public:
    LabelParser(string labelFilePath);
    vector<unordered_map<string, string> > parseObject(string objectType);
    string parseRootProperty(string propertyName);
    ~LabelParser();
    string getLabelContent() const;
    void setLabelContent(const string &value);
    string getOriginalLabelContent() const;
    void setOriginalLabelContent(const string &value);
};

#endif // LABELPARSER_H
