/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef HELPER_H
#define HELPER_H

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>

#include "types.h"
#include  <climits>
#include "detectorexceptions.h"
#include "easyloggingcpp/easylogging++.h"

/**
 * <p>
 * This class contains helper function.
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */

using namespace std;


class Helper
{
public:
    Helper();

    static inline bool startsWith(const string& value, const string& starting) {
        return starting.length() <= value.length()
            && equal(starting.begin(), starting.end(), value.begin());
    }

    static inline bool endsWidth(std::string const & value, std::string const & ending){
        if (ending.size() > value.size()) return false;
            return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    // trim from start
    static inline std::string ltrim(std::string s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
            return s;
    }

    // trim from end
    static inline std::string rtrim(std::string s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
            return s;
    }

    static inline string trim(string s){
        return ltrim(rtrim(s));
    }

    static inline string  trimCharacters(string const &value, string const & left, string const & right){
        string ret = value;
        if(startsWith(value, left)){
            ret = value.substr(left.size());
        }

        if(endsWidth(ret, right)){
            ret = ret.substr(0, ret.size()-right.size());
        }

        return ret;
    }

    static inline string trimQuote(string  s){

        if(startsWith(s, "\"")){
            s = s.substr(1);
        }
        if(endsWidth(s, "\"")){
            s = s.substr(0, s.size()-1);
        }

        return s;
    }

    static inline std::string replace(std::string str, const std::string& from, const std::string& to) {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    }

    static inline string toUpperCase(string str){
        std::transform(str.begin(), str.end(),str.begin(), ::toupper);
        return str;
    }

    static inline bool iequals(string left, string right){

        return toUpperCase(left) == toUpperCase(right);
    }

    static inline PdsImagePixelType toPdsImagePixelType(string s){

        if(toUpperCase(s) == "PC_REAL"){
            return PC_REAL;
        } else if(toUpperCase(s) == "SUN_INTEGER"){
            return SUN_INTEGER;
        } else if(toUpperCase(s) == "UNKNOWN_TYPE"){
            return UNKNOWN_TYPE;
        }

        return  UNKNOWN_TYPE;

    }

    static inline vector<char> readBinaryFile(string fileName){
        std::ifstream input(fileName, std::ios::binary );
            // copies all data into buffer
            std::vector<char> buffer((
                    std::istreambuf_iterator<char>(input)),
                    (std::istreambuf_iterator<char>()));

            return buffer;
    }

    static std::vector<char> ReadAllBytes(string filename)
    {
        ifstream ifs(filename, ios::binary|ios::ate);

        if(!ifs.is_open() || !ifs.good()){
            LOG(ERROR) << "Access Error when opening file " + filename << endl;
            throw DetectorException("Access Error when opening file " + filename);
        }
        ifstream::pos_type pos = ifs.tellg();

        std::vector<char>  result(pos);

        ifs.seekg(0, ios::beg);
        ifs.read(&result[0], pos);

        return result;
    }


    static inline int getBinaryFileSize(string filename){
        ifstream file( filename, ios::binary | ios::ate);
        if(!file.is_open() || !file.good()){
            LOG(ERROR) << "Access Error when opening file " + filename << endl;
            throw DetectorException("Access Error when opening file " + filename);
        }
        return file.tellg();
    }

    static inline float bytesToFloat(unsigned char b[]){
        unsigned char t = b[0];
            b[0] = b[3];
            b[3] = t;
            t = b[1];
            b[1] = b[2];
            b[2] = t;
        float f;
        memcpy(&f, &b, sizeof(f));
        float g = reinterpret_cast<float&>(b);
        uint32_t ii = reinterpret_cast<uint32_t&>(b);
        uint32_t rr = reverseBits(ii);

        f = reinterpret_cast<float&>(rr);
        return f;
    }

    static inline uint32_t reverseBits(uint32_t v){

       // unsigned int v;     // input bits to be reversed
        uint32_t r = v; // r will be reversed bits of v; first get LSB of v
        int s = sizeof(v) * CHAR_BIT - 1; // extra shift needed at end

        for (v >>= 1; v; v >>= 1)
        {
          r <<= 1;
          r |= v & 1;
          s--;
        }
        r <<= s; // shift when v's highest bits are zero

        return r;
    }


    static inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }


    static inline std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, elems);
        return elems;
    }

    static inline vector<string> readTextFile(string path)
        {
            ifstream in(path);
            if(!in.is_open() || !in.good()){
                LOG(ERROR) << "Access Error when opening file " + path << endl;
                throw DetectorException("Access Error when opening file " + path);
            }
            string line;
            vector<string> res;

            while(getline(in, line)){

                res.push_back(line);
            }

            return res;
        }



    ~Helper();
};

#endif // HELPER_H
