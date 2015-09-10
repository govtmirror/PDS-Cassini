/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

/**
 * <p>
 * This class is the main detector.
 *
 * Header file
 * </p>
 *
 * @author TCSASSEMBLER, yedtoss
 * @version 1.1
 */

#include "common.h"
#include "randomforest.h"
#include "detectorutility.h"
#include "easyloggingcpp/easylogging++.h"

#include <mutex>

class PropellerDetector {

private:
    friend class cereal::access;

    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( trainIds);
      }
public:

	VVD trainData;
	VD trainResults;
	VVD testData;
	VS trainIds;
	VS testIds;
	VI trainGroupIds;
	VI testGroupIds;
    vector<TransformerData> testTransformers;


	PropellerDetector() {
		currentImageId = 0;
		trainData.clear();
		trainResults.clear();
		testData.clear();
		trainIds.clear();
		testIds.clear();

		//startTimeProgram = getTime();
	}

	mutex mtx_train;

	int trainingData(VD &imageData, string imageId, string startTime, double declination, double rightAscension, double twistAngle, VD &scPlanetPositionVector, string instrumentId, string instrumentModeId, VS &imageGroundTruth, int cImageId) {
		//double xtime = getTime(),etime;

		//gImageId = imageId;
		//{VS v = splt(imageGroundTruth[0], ',');
		//gY = (int)round(atof(v[1].c_str()));
		//gX = (int)round(atof(v[2].c_str()));}
		//const int cImageId = currentImageId;
		VVD features = solve(imageData, imageId, startTime, declination, rightAscension, twistAngle, scPlanetPositionVector, instrumentId, instrumentModeId, cImageId);

		//etime = getTime()-xtime;
		//DB(etime);

		mtx_train.lock();
		if(cImageId >= trainIds.size())
			trainIds.resize(cImageId + 1);
		//trainIds.PB(imageId);
		currentImageId++;

		VI gtfx, gtfy;
		for (string s : imageGroundTruth) {
			VS v = splt(s, ',');
			gtfy.PB((int)round(atof(v[1].c_str())));
			gtfx.PB((int)round(atof(v[2].c_str())));
		}

		//mtx_train.lock();
		REP(i, features.S) {
			int x = (int)features[i][1];
			int y = (int)features[i][2];
			bool correct = false;
			REP(j, gtfx.S) correct |= sqrt((gtfx[j] - x) * (gtfx[j] - x) + (gtfy[j] - y) * (gtfy[j] - y)) < CORRECT_DIST;

			features[i][3] = correct;
			trainData.PB(features[i]);
			trainResults.PB(correct);
		}
		mtx_train.unlock();

		//etime = getTime()-xtime;
		//DB(etime);

		return 0;
	}

	int testingData(VD &imageData, string imageId, string startTime, double declination, double rightAscension, double twistAngle, VD &scPlanetPositionVector, string instrumentId, string instrumentModeId, int cImageId) {

		//double xtime = getTime();

		//{gImageId = "t"+imageId;
		//gY = (int)-100;
		//gX = (int)-100;}
		//const int cImageId = currentImageId;

		VVD features = solve(imageData, imageId, startTime, declination, rightAscension, twistAngle, scPlanetPositionVector, instrumentId, instrumentModeId, cImageId);

		mtx_train.lock();

		//TODO: FIX ME, VERY BAD
		int idx = cImageId - trainIds.S;
		if(idx >= testTransformers.size())
			testTransformers.resize(idx + 1);
        testTransformers[idx] = TransformerData(instrumentId, instrumentModeId, rightAscension, declination, twistAngle, scPlanetPositionVector);

		REP(i, features.S) testData.PB(features[i]);

		//testIds.PB(imageId);
		if(cImageId - currentImageId >= testIds.size())
			testIds.resize(cImageId - currentImageId + 1);
		testIds[cImageId - currentImageId] = imageId;
		//currentImageId++;

		mtx_train.unlock();

		//xtime = getTime()-xtime;
		//DB(xtime);

		return 0;
	}

    RandomForest train(Options opt){

        //double xtime = getTime(),dtime;
        trainGroupIds = createGroups(trainIds);
        //print(trainGroupIds);

        RandomForestConfig cfg;
        cfg.featuresIgnored = 4;
        cfg.randomFeatures = {4};
        cfg.randomPositions = {5};
        cfg.maxNodeSize = 10;
        cfg.timeLimit = opt.time_limit>=0 ? 60*opt.time_limit : 60*55;
        cfg.threadsNo = opt.num_threads > 0 ? opt.num_threads : THREADS_NO;
        cfg.treesNo = opt.max_trees > 0 ? opt.max_trees : 400;

        RandomForest RF;
        RF.train(trainData, trainResults, cfg);

        //dtime = getTime()-xtime;
        //DB(dtime);

        return RF;

    }

    struct DetectionStruct
    {
		double importance;
		int imageId;
		int x, y;
		int groupId;
		double rad;
		double lont;
    };

    VS test(RandomForest& RF, Options opt){

        ofstream fw(opt.log_file_cont);

        RF.config.threadsNo = opt.num_threads > 0 ? opt.num_threads : THREADS_NO;
        //double xtime = getTime(),dtime;

        LOG(INFO) << "Test - creating groups.\n";

        testGroupIds = createGroups(testIds);
        //print(testGroupIds);

        //dtime = getTime()-xtime;
        //DB(dtime);

		LOG(INFO) << "Test - making predictions.\n";

        VC<pair<double, int>> vp;
        REP(i, testData.S) vp.PB(MP(RF.predict(testData[i]), i));
        sort(vp.rbegin(), vp.rend());

        //VI detectionsUsed[MAX_ID];
        vector< vector< int > > detectionsUsed;

        /*vector< Transformer > tempTransformers;
		for(int i = 0; i < testIds.S; i++)
		{
			tempTransformers.push_back(Transformer(testTransformers[i]));
		}*/

		vector< DetectionStruct > detection_list;

		LOG(INFO) << "Test - looping candidates.\n";

        VS rv;
        REP(i, vp.S) {
            if (rv.S >= opt.max_candidates) break;

            int id = vp[i].Y;

            DetectionStruct detection;

			detection.importance = vp[i].X;
            detection.imageId = testData[id][0];
            detection.x = testData[id][1];
            detection.y = testData[id][2];

			if(detection.imageId >= detectionsUsed.size())
			{
				detectionsUsed.resize(detection.imageId + 1);
			}

            bool ok = true;
            for (int d : detectionsUsed[detection.imageId]) {
                ok &= sqrt((testData[d][1] - detection.x) * (testData[d][1] - detection.x) + (testData[d][2] - detection.y) * (testData[d][2] - detection.y)) > MIN_DIST;
            }
            if (!ok) continue;

            detectionsUsed[detection.imageId].PB(id);
            /*detection.rad = 0;
            detection.lont = 0;
            int idx = detection.imageId - trainIds.S;

            if(testTransformers.size() > idx){

                Transformer(testTransformers[idx]).pixelPosition2RadiusLongitude(detection.x, detection.y, detection.rad, detection.lont);
            }
            else
            {
				LOG(ERROR) << "MAXIMUM FAIL\n";
            }*/
            /*rv.PB(testIds[idx] + "," + i2s(detection.y) + "," + i2s(detection.x) + "," + i2s(testGroupIds[idx] + 1) +  "," + i2s(detection.rad) + "," + i2s(detection.lont));
            // Print result continuously
            //LOG(INFO) << rv[rv.size()-1]<<endl;
            fw<<rv[rv.size()-1]<<endl;*/

            detection_list.PB(detection);
        }

		LOG(INFO) << "Test - sort by id.\n";

        //Sort by image id
        sort(detection_list.begin(), detection_list.end(),
			[](const DetectionStruct& a, const DetectionStruct& b) -> bool
			{
				return a.imageId < b.imageId;
			});

		LOG(INFO) << "Test - loop by id.\n";

		int lastImageId = -1;
		Transformer* tr = 0;//(testTransformers.front());
		for(DetectionStruct& ds : detection_list)
		{
			int idx = ds.imageId - trainIds.S;
			if(ds.imageId != lastImageId)
			{
				if(tr)
					delete tr;
				tr = new Transformer(testTransformers[idx]);
			}
			double rad = 0, lont = 0;
			tr->pixelPosition2RadiusLongitude(ds.x, ds.y, rad, lont);
			ds.rad = rad;
			ds.lont = lont;
			lastImageId = ds.imageId;
		}
		delete tr;

		LOG(INFO) << "Test - sort by importance.\n";

		//Sort by importance
		sort(detection_list.begin(), detection_list.end(),
			[](const DetectionStruct& a, const DetectionStruct& b) -> bool
			{
				return a.importance > b.importance;
			});

		LOG(INFO) << "Test - loop by importance.\n";

		for(const DetectionStruct& ds : detection_list)
		{
			int idx = ds.imageId - trainIds.S;
			rv.PB(testIds[idx] + "," + i2s(ds.y) + "," + i2s(ds.x) + "," + i2s(testGroupIds[idx] + 1) +  "," + i2s(ds.rad) + "," + i2s(ds.lont) + "," + i2s(ds.importance));
            fw << rv[rv.size()-1] << endl;
		}

		LOG(INFO) << "Test - return.\n";

        //DB(rv.S);
        //dtime = getTime()-xtime;
        //DB(dtime);
        return rv;

    }



};

