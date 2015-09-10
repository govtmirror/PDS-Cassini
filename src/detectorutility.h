/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#ifndef DETECTOR_UTILITY_H
#define DETECTOR_UTILITY_H


#include "simplematrix.h"
#include "common.h"
#include "types.h"
#include "transformer.h"
#include "ringsubtractor.h"

/**
 * <p>
 * This file contains utility functions.
 *
 * Header file
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */


double getTime() {
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

#ifdef USE_DRAW
typedef unsigned char UC;
void draw_bmp(VVD img) {
    string name = gImageId;

    UC imbuf[1024*1024];
    REP(y,1024) REP(x,1024) {
        imbuf[x+1024*(1023-y)] = (img[y][x]==0)?255:0;
    }
    RingSubtractor::normalize(img);
    RingSubtractor::elementaryScale(img,255);
    for(int x = 0; x < 1024; x++) for(int y = 0; y < 1024; y++) {
        double rgbVal = img[y][x];
        int sz = 20;

        if(y > gY+sz && y < gY+sz+3 && x>gX-sz && x<gX+sz) rgbVal = 0;
        if(y > gY-sz-3 && y < gY-sz && x>gX-sz && x<gX+sz) rgbVal = 0;
        if(y > gY-sz && y < gY+sz && x>gX+sz && x<gX+sz+3) rgbVal = 0;
        if(y > gY-sz && y < gY+sz && x>gX-sz-3 && x<gX-sz) rgbVal = 0;
        if(rgbVal==0) imbuf[x+1024*(1023-y)] = 0;

        img[y][x] = rgbVal;
    }
    REP(y,1024) REP(x,1024) {
        if(!imbuf[x+1024*(1023-y)]) imbuf[x+1024*(1023-y)] = img[y][x];
    }
    char s_name[100] = "";
    sprintf(s_name,"(cpp)%03s.bmp",name.c_str());
    gen_gray_bmp(s_name,1024,1024,imbuf,1024*1024);
}
#else
void draw_bmp(VVD &img) {
}
#endif


VVD medianFilter_avg(VVD &img, int size) {
    VVD rv(1024, VD(1024, 0));
    double data[500];
    double avg = 0;

    REP(y, 1024) REP(x, 1024) {
        avg = 0;
        int no = 0;
        FOR(dy, -size, +size + 1) FOR(dx, -size, +size + 1) {
            int ny = y + dy;
            int nx = x + dx;
            if (ny >= 0 && ny < 1024 && nx >= 0 && nx < 1024) {
                avg += img[ny][nx];
                no ++;
            }
        }
        if(no==0) no = 1;
        rv[y][x] = avg/no;
    }
    return rv;
}

VVD medianFilter_nth(VVD &img, int size) {
    VVD rv(1024, VD(1024, 0));
    double data[500];

    REP(y, 1024) REP(x, 1024) {
        int no = 0;
        FOR(dy, -size, +size + 1) FOR(dx, -size, +size + 1) {
            int ny = y + dy;
            int nx = x + dx;
            if (ny >= 0 && ny < 1024 && nx >= 0 && nx < 1024)
                data[no++] = img[nx][ny];
        }
        nth_element(data, data + no / 2, data + no);
        rv[x][y] = data[no/2];
    }
    return rv;
}

void calculateBag(VVD &img,VVD &radius, double minRadius, double maxRadius, double* out_allVar, double* bagSum, double* bagSum2, double* bagVar, double* bagSumPos, double* bagSumNeg, int* bagSumTot, int* bagSumPosTot, int* bagSumNegTot) {
    double allSum = 0;
    double allSum2 = 0;
    double allVar = 0;
    int allTot = 0;
    REP(i, RADIUS_BAGS) {
        bagSum[i] = 0;
        bagSum2[i] = 0;
        bagVar[i] = 0;
        bagSumPos[i] = 0;
        bagSumNeg[i] = 0;
        bagSumTot[i] = 0;
        bagSumPosTot[i] = 0;
        bagSumNegTot[i] = 0;
    }
    REP(y, 1024) REP(x, 1024) {
        int bag = min(RADIUS_BAGS - 1.0, max(0.0, (radius[y][x] - minRadius) / (maxRadius - minRadius) * RADIUS_BAGS));
        allSum += img[y][x];
        allSum2 += img[y][x]*img[y][x];
        allTot++;
        bagSum[bag] += img[y][x];
        bagSum2[bag] += img[y][x]*img[y][x];
        bagSumTot[bag]++;
        if (img[y][x] > 0) {
            bagSumPos[bag] += img[y][x];
            bagSumPosTot[bag]++;
        }
    }

    allVar = (allSum2 - allSum*allSum/allTot) / allTot;
    REP(i, RADIUS_BAGS) {
        bagSumNeg[i] = bagSum[i] - bagSumPos[i];
        bagSumNegTot[i] = bagSumTot[i] - bagSumPosTot[i];
        if (bagSumTot[i]) bagVar[i] = (bagSum2[i] - bagSum[i]*bagSum[i]/bagSumTot[i]) / bagSumTot[i];
        if (bagSumTot[i]) bagSum[i] /= bagSumTot[i];
        if (bagSumPosTot[i]) bagSumPos[i] /= bagSumPosTot[i];
        if (bagSumNegTot[i]) bagSumNeg[i] /= bagSumNegTot[i];
    }

    *out_allVar = allVar;
}


VVD findObjects(VVD &img, VVD &radius, double minRadius, double maxRadius, double* out_allVar, double* bagSum, double* bagSum2, double* bagVar, double* bagSumPos, double* bagSumNeg, int* bagSumTot, int* bagSumPosTot, int* bagSumNegTot, double mv = 0.0, double mv2 = 0.00) {
    calculateBag(img,radius, minRadius, maxRadius, out_allVar, bagSum, bagSum2, bagVar, bagSumPos, bagSumNeg, bagSumTot, bagSumPosTot, bagSumNegTot);

    VVD rv(1024, VD(1024, 0));

    REP(y, 1024) REP(x, 1024) {
        int bag = (int)min(RADIUS_BAGS - 1.0, max(0.0, (radius[y][x] - minRadius) / (maxRadius - minRadius) * RADIUS_BAGS));
        double var = sqrt(bagVar[bag]);
        double mean = bagSum[bag];
        double val = img[y][x];
        int size = 1;
        double phi_threshold = 3.5;
        if( (val-mean)/var < -phi_threshold ) {
            for(int dy = -size; dy < size+1; dy++) {
                for(int dx = -size; dx < size+1; dx++) {
                    int nx,ny; ny = y+dy; nx = x+dx;
                    if (ny >= 0 && ny < 1024 && nx >= 0 && nx < 1024) {

                        if(ny>y && img[ny][nx]>img[ny-1][x]) {
                            if(nx > x && img[ny][nx]>img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]>img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                        if(ny<y && img[ny][nx]>img[ny+1][x]) {
                            if(nx > x && img[ny][nx]>img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]>img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                        if(ny == y) {
                            if(nx > x && img[ny][nx]>img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]>img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                    }
                }
            }
        }
        if( (val-mean)/var > phi_threshold ) {
            for(int dy = -size; dy < size+1; dy++) {
                for(int dx = -size; dx < size+1; dx++) {
                    int nx,ny; ny = y+dy; nx = x+dx;
                    if (ny >= 0 && ny < 1024 && nx >= 0 && nx < 1024) {

                        if(ny>y && img[ny][nx]<img[ny-1][x]) {
                            if(nx > x && img[ny][nx]<img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]<img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                        if(ny<y && img[ny][nx]<img[ny+1][x]) {
                            if(nx > x && img[ny][nx]<img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]<img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                        if(ny == y) {
                            if(nx > x && img[ny][nx]<img[ny][nx-1]) rv[ny][nx] = img[ny][nx];
                            if(nx < x && img[ny][nx]<img[ny][nx+1]) rv[ny][nx] = img[ny][nx];
                            if(nx == x) rv[ny][nx] = img[ny][nx];
                        }
                    }
                }
            }
        }
    }
    REP(y, 1024) REP(x, 1024) img[y][x] = rv[y][x];

    REP(y, 1024) REP(x, 1024) rv[y][x] = 0;
    REP(y, 1023) REP(x, 1023) {
        int sign = (img[y][x] != 0?1:0) + (img[y][x+1] != 0?1:0) + (img[y+1][x] != 0?1:0) + (img[y+1][x+1] != 0?1:0);
        if (sign != 4) continue;
        double sum = img[y][x] + img[y][x+1] + img[y+1][x] + img[y+1][x+1];
        if (abs(sum) <= 0) continue;
        rv[y+0][x+0] = img[y+0][x+0];
        rv[y+0][x+1] = img[y+0][x+1];
        rv[y+1][x+0] = img[y+1][x+0];
        rv[y+1][x+1] = img[y+1][x+1];
    }
    REP(y, 1024) REP(x, 1024) if(abs(rv[y][x])<mv2) rv[y][x] = 0;
    draw_bmp(rv);

    return rv;
}

VVD generateDetections(VVD &img, VVD &obj, VVD &radius, VVD &longitude, double minRadius, double maxRadius, double allVar, double* bagSum, double* bagSum2, double* bagVar, double* bagSumPos, double* bagSumNeg, int* bagSumTot, int* bagSumPosTot, int* bagSumNegTot, int cImageId, double mv = 0.00) {
    VVI vs(1024, VI(1024, 0));

    VVD rv;

    REP(y, 1024) REP(x, 1024) {
        if (vs[y][x] || abs(obj[y][x]) < mv) continue;

        queue<int> q;
        q.push(x);
        q.push(y);
        int sign = obj[y][x] > 0;

        double sum = 0;
        double sum2 = 0;
        double sumx = 0;
        double sumy = 0;
        double sumrad = 0;
        double sumlon = 0;
        double minrad = 1e9;
        double maxrad = -1e9;
        double minlon = 1e9;
        double maxlon = -1e9;
        int tot = 0;
        while (!q.empty()) {
            int cx = q.front(); q.pop();
            int cy = q.front(); q.pop();
            if (cx < 0 || cx >= 1024 || cy < 0 || cy >= 1024 || vs[cy][cx] || abs(obj[cy][cx]) < mv || sign != (obj[cy][cx] > 0)) continue;
            vs[cy][cx] = 1;

            sum += obj[cy][cx];
            sum2 += obj[cy][cx] * obj[cy][cx];
            sumx += cx;
            sumy += cy;
            sumrad += radius[cy][cx];
            sumlon += longitude[cy][cx];
            minrad = min(minrad, radius[cy][cx]);
            maxrad = max(maxrad, radius[cy][cx]);
            minlon = min(minlon, longitude[cy][cx]);
            maxlon = max(maxlon, longitude[cy][cx]);
            tot++;

            q.push(cx+1); q.push(cy);
            q.push(cx-1); q.push(cy);
            q.push(cx); q.push(cy+1);
            q.push(cx); q.push(cy-1);
        }

        if (tot > 100) continue;
        if (tot <= 3) continue;

        double var = (sum2-(sum*sum)/tot)/tot;
        sum /= tot;
        sum2 /= tot;
        sumx /= tot;
        sumy /= tot;
        sumrad /= tot;
        sumlon /= tot;
        int bag = min(RADIUS_BAGS - 1., max(0., (sumrad - minRadius) / (maxRadius - minRadius) * RADIUS_BAGS));

        VD feat;
        feat.PB(cImageId);
        feat.PB(sumx);
        feat.PB(sumy);
        feat.PB(0);

        feat.PB(abs(sum));
        feat.PB(sum);
        feat.PB(sum2);
        feat.PB(bagVar[bag]);
        feat.PB(bagVar[bag]/allVar);
        feat.PB(allVar);
        feat.PB(sumrad);
        feat.PB(tot);
        feat.PB(bagSum[bag]);
        feat.PB(bagSum[bag] - sum);
        feat.PB(bagSumPos[bag]);
        feat.PB(bagSumNeg[bag]);
        feat.PB(maxrad - minrad);
        feat.PB(maxlon - minlon);
        rv.PB(feat);

    }
    //DB(rv.S);

    return rv;
}

VI createGroups(VS &ids) {
    const int MAX_DIFF = 500;
    VI rv(ids.S, -1);
    VC<pair<LL,int>> vp;
    REP(i, ids.S) {
        string s = ids[i];
        if (s.S < 1) continue;
        s = s.substr(1);
        bool ok = true;
        for (char c : s) ok &= isdigit(c);
        if (!ok) continue;
        LL id = atoll(s.c_str());
        if (id == 0) continue;
        vp.PB(MP(id, i));
    }
    sort(ALL(vp));

    int curId = -1;
    REP(i, vp.S) {
        if (i == 0 || vp[i].X - vp[i-1].X > MAX_DIFF) curId++;
        rv[vp[i].Y] = curId;
    }
    for (int &x : rv) if (x == -1) x = curId++;
    return rv;
}

VVD solve(VD &imageData, string imageId, string startTime, double declination, double rightAscension, double twistAngle, VD &scPlanetPositionVector, string instrumentId, string instrumentModeId, int cImageId) {
    //double xtime,objects_time;

    //xtime = getTime();
    Transformer transformer(instrumentId, instrumentModeId, rightAscension, declination, twistAngle, scPlanetPositionVector);


    VVD image(1024, VD(1024, 0));
    REP(x, 1024) REP(y, 1024) image[y][x] = imageData[x + 1024 * y];

    RingSubtractor sub;

    VVD subtracted = sub.subtractRings(image, transformer);


    VVD radius = transformer.getRadiusArray(true);
    VVD longitude = transformer.getRadiusArray(true);
    //objects_time = getTime()-xtime;	DB(objects_time);

    double minRadius = 1e9;
    double maxRadius = 0;
    for (VD &v : radius) for (double d : v) {
        minRadius = min(minRadius, d);
        maxRadius = max(maxRadius, d);
    }

    // VVD subtracted = image;
    VVD median = medianFilter_nth(subtracted, 4);
    VVD objects(1024, VD(1024));
    REP(y, 1024) REP(x, 1024) objects[y][x] = subtracted[y][x] - median[y][x];
    REP(y, 1024) REP(x, 1024) if(y<6||x<6||y>1023-6 || x>1023-6) objects[y][x] = 0;

	double bagSum[RADIUS_BAGS] = {0};
	double bagSum2[RADIUS_BAGS] = {0};
	double bagVar[RADIUS_BAGS] = {0};
	double bagSumPos[RADIUS_BAGS] = {0};
	double bagSumNeg[RADIUS_BAGS] = {0};
	int bagSumTot[RADIUS_BAGS] = {0};
	int bagSumPosTot[RADIUS_BAGS] = {0};
	int bagSumNegTot[RADIUS_BAGS] = {0};

	double i_allVar;
    objects = findObjects(objects, radius, minRadius, maxRadius, &i_allVar, bagSum, bagSum2, bagVar, bagSumPos, bagSumNeg, bagSumTot, bagSumPosTot, bagSumNegTot);

    const VVD & rvv = generateDetections(image, objects, radius, longitude, minRadius, maxRadius, i_allVar, bagSum, bagSum2, bagVar, bagSumPos, bagSumNeg, bagSumTot, bagSumPosTot, bagSumNegTot, cImageId);

    return rvv;
}


#endif // DETECTOR_UTILITY_H

