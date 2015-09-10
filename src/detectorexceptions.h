/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef DETECTOREXCEPTIONS_H
#define DETECTOREXCEPTIONS_H

/**
* <p>
* This class defined custom exceptions used in the code.
*
* Header file
* </p>
*
* @author yedtoss
* @version 1.0
*/


#include <exception>
#include <string>

using namespace std;


class LabelParseException: public exception
{
    string s;
 public:
  LabelParseException(string message) : s(message) {}

  virtual const char* what() const throw()
  {
    return s.c_str();
  }
};


class DetectorException: public exception
{
    string s;
 public:
  DetectorException(string message) : s(message) {}

  virtual const char* what() const throw()
  {
    return s.c_str();
  }
};
#endif // DETECTOREXCEPTIONS_H

