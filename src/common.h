#ifndef COMMON_H
#define COMMON_H


#define TIMERS
//#include "draw.h"


#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <sys/time.h>
#include <assert.h>
#include <thread>
//#include <cmath>
#include <math.h>
#include <queue>
#include <stack>
//#include <emmintrin.h>
#include   <mutex>
#include "cereal/archives/binary.hpp"
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/utility.hpp>
#include "types.h"
#include "easyloggingcpp/easylogging++.h"


/**
 * <p>
 * This file contains common inclusion and defines.
 *
 * Header file
 * </p>
 *
 * @author TCSASSEMBLER, yedtoss
 * @version 1.1
 */

#ifdef LOCAL
int THREADS_NO = 3;
#else
int THREADS_NO = 1;
#endif

const int MAX_RESULTS = 10000;
const int MAX_ID = 1000;
const double CORRECT_DIST = 7.0;
const double MIN_DIST = 7.0;

using namespace std;

#define INLINE   inline __attribute__ ((always_inline))
#define NOINLINE __attribute__ ((noinline))

#define ALIGNED __attribute__ ((aligned(16)))

#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define SSELOAD(a)     _mm_load_si128((__m128i*)&a)
#define SSESTORE(a, b) _mm_store_si128((__m128i*)&a, b)

#define FOR(i,a,b)  for(int i=(a);i<(int)(b);++i)
#define REP(i,a)    FOR(i,0,a)
#define ZERO(m)     memset(m,0,sizeof(m))
#define ALL(x)      x.begin(),x.end()
#define PB          push_back
#define S           size()
#define LL          long long
#define ULL         unsigned long long
#define LD          long double
#define MP          make_pair
#define X           first
#define Y           second
#define VC          vector
#define PII         pair <int, int>
#define PDD         pair <double, double>
#define VI          VC < int >
#define VVI         VC < VI >
#define VVVI        VC < VVI >
#define VPII        VC < PII >
#define VD          VC < double >
#define VVD         VC < VD >
#define VVVD        VC < VVD >
#define VS          VC < string >
#define VVS         VC < VS >
#define DB(a)       LOG(INFO) << #a << ": " << (a) << endl;
#define ASSERT      /*assert*/

template<class T> void print(VC < T > v) {string str;  str = "["; if (v.S) str += to_string(v[0]);FOR(i, 1, v.S) str += ", " + to_string(v[i]); str+= "]";   LOG(INFO) << str << endl;}
template<class T> string i2s(T x) {return to_string(x);}
VS splt(string s, char c = ' ') {VS all; int p = 0, np; while (np = s.find(c, p), np >= 0) {if (np != p) all.PB(s.substr(p, np - p)); p = np + 1;} if (p < s.S) all.PB(s.substr(p)); return all;}


struct TreeNode {
    int level;
    int feature;
    double value;
    double result;
    int left;
    int right;


    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( level, feature, value, result, left, right);
      }

    TreeNode() {
        level = -1;
        feature = -1;
        value = 0;
        result = 0;
        left = -1;
        right = -1;
    }
};

struct RandomForestConfig {
    static const int MSE = 0;
    static const int MCE = 1;
    static const int MAE = 2;
    static const int CUSTOM = 3; // Custom

    //Grouping
    int groupFeature = -1;
    VI groups = {};
    double bagSize = 1.0;

    //Tree construction
    VI randomFeatures = {5};
    VI randomPositions = {2};
    int maxNodeSize = 1;
    int maxLevel = 100;

    //Feature Selection
    int featuresIgnored = 0;
    VI featuresUsed = {};

    int threadsNo = 1;
    int treesNo = 1;
    double timeLimit = 60 * 55;
    int lossFunction = MSE;
    bool useBootstrapping = true;
    bool computeImportances = false;
    bool computeOOB = false; //NOT IMPLEMENTED

    //Boosting
    bool useLineSearch = false;
    double shrinkage = 0.1;

    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( groupFeature, groups, bagSize, randomFeatures, randomPositions, maxNodeSize, maxLevel, featuresIgnored, featuresUsed,
            threadsNo, treesNo, timeLimit, lossFunction, useBootstrapping, computeImportances, computeOOB, useLineSearch, shrinkage);
      }
};


#endif // COMMON_H

