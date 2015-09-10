/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef SIMPLEMATRIX_H
#define SIMPLEMATRIX_H

#include "common.h"
#include "types.h"



/**
 * <p>
 * This class defined custom matrix type.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */
class SimpleMatrix {

public:
    VD data;
    int numRows = 0;
    int numCols = 0;

public:
    SimpleMatrix() { }

    SimpleMatrix(int numRows, int numCols) {
        this->numRows = numRows;
        this->numCols = numCols;
        this->data = VD(numRows * numCols);
    }

    SimpleMatrix(VVD data) {
        this->numRows = data.S;
        this->numCols = data[0].S;
        this->data = VD(numRows * numCols);

        int pos = 0;
        REP(row, numRows) REP(col, numCols) this->data[pos++] = data[row][col];
    }

    SimpleMatrix mult(SimpleMatrix &b) {
        assert(numCols == b.numRows);
        SimpleMatrix ret = SimpleMatrix(numRows, b.numCols);
        VD &dataA = data;
        VD &dataB = b.data;
        VD &dataRet = ret.data;

        for (int i = 0; i < ret.numRows; i++) {
            for (int j = 0; j < ret.numCols; j++) {
                double sum = 0;
                int idxA = i * numRows;
                int idxB = j;
                for (int k = 0; k < numCols; k++) {
                    sum += dataA[idxA++] * dataB[idxB];
                    idxB += b.numCols;
                }
                dataRet[i * ret.numCols + j] = sum;
            }
        }

        return ret;
    }

    SimpleMatrix plus(SimpleMatrix &b) {
        assert(numRows == b.numRows && numCols == b.numCols);
        SimpleMatrix ret = SimpleMatrix(numRows, numCols);
        REP(i, numRows * numCols) ret.data[i] = data[i] + b.data[i];
        return ret;
    }

    SimpleMatrix minus(SimpleMatrix &b) {
        assert(numRows == b.numRows && numCols == b.numCols);
        SimpleMatrix ret = SimpleMatrix(numRows, numCols);
        REP(i, numRows * numCols) ret.data[i] = data[i] - b.data[i];
        return ret;
    }

    SimpleMatrix transpose() {
        SimpleMatrix ret = SimpleMatrix(numCols, numRows);
        REP(row, numRows) REP(col, numCols) ret.data[col * numRows + row] = data[row * numCols + col];
        return ret;
    }

    SimpleMatrix scale(double scale) {
        SimpleMatrix ret = SimpleMatrix(numRows, numCols);
        REP(i, numRows * numCols) ret.data[i] = data[i] * scale;
        return ret;
    }

    int getNumRows() {
        return numRows;
    }

    int getNumCols() {
        return numCols;
    }

    double get(int i, int j) {
        return data[i * numCols + j];
    }
};



#endif // SIMPLEMATRIX_H

