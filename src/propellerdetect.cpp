/**
 * Copyright (C) 2015 TopCoder Inc., All Rights Reserved.
 */

#include "propellerdetect.h"

#include "propellerdetector.h"

#include "helper.h"
#include "indextabparser.h"
#include "pdsimagereader.h"

#include "cereal/archives/binary.hpp"

#include <fstream>
#include <unordered_map>

#include "common.h"
#include <atomic>

#include "easyloggingcpp/easylogging++.h"
#include <iomanip>


/**
 * <p>
 * This class expose routines used to detect propeller.
 * </p>
 *
 * @author yedtoss
 * @version 1.0
 */
PropellerDetect::PropellerDetect()
{

}

typedef struct ScoreResult{

    vector<pair<int, vector<string> > > baskets;
    int groundTruth_size;
    int answer_size;
    int numDetected;
    double positionScore;

    template <class Archive>
      void serialize( Archive & ar )
      {
        ar( baskets, groundTruth_size, answer_size, numDetected, positionScore);
      }


} ScoreResult;


const double PropellerDetect::POSITION_TOLERANCE = 10.0;

//Ground truth token positions
const int PropellerDetect::TOKEN_IMAGE = 0;
const int PropellerDetect::TOKEN_LINE = 1;
const int PropellerDetect::TOKEN_SAMPLE = 2;
const int PropellerDetect::TOKEN_NICKNAME = 15;

mutex values_mutex;

vector<PdsImageParameters> PropellerDetect::readLabelFile(string labelFilePath, Options opt){

    //LOG(DEBUG) << "Entering  vector<PdsImageParameters> PropellerDetect::readLabelFile(string labelFilePath)"<<endl;

    LOG(INFO) << "Processing label file "<< labelFilePath << endl;
    vector<PdsImageParameters> results;

    IndexTabParser indexTabParser(labelFilePath);
    string tabFilePath = indexTabParser.getTabFilePath();

    LOG(INFO) << "Processing tab file "<< tabFilePath << endl;

    vector<string> tabLines = Helper::readTextFile(tabFilePath);

    int num_thread = opt.num_threads > 0 ? opt.num_threads : THREADS_NO;;

    auto f = [&results, &tabLines, &indexTabParser, &tabFilePath] (int start_idx, int end_idx, int num) {

        vector<PdsImageParameters> partial_res;
        for(int i=start_idx; i< end_idx; i++){
            string line = tabLines[i];

            try{
                if(!Helper::trim(line).empty()){
                    PdsImageParameters params = indexTabParser.parseTabLine(line);
                    partial_res.push_back(params);
                }

            } catch(...){
                // Don't rethrow the error as we can continue processing
                LOG(ERROR)<< "Error parsing line " + to_string(i+1) + " of " + "tab file " + tabFilePath<< endl;
            }

        }

        {
            // Putting this in his own scope
            std::lock_guard<std::mutex> lk(values_mutex);
            results.insert(results.end(), partial_res.begin(), partial_res.end());
            // values_mutex automatically released even in case of exception
        }

    };


    for(int i=0; i<num_thread; i++){
        int start_idx = i*(tabLines.size()/num_thread);
        int end_idx = (i < num_thread -1) ? start_idx + tabLines.size()/num_thread : tabLines.size();
        std::thread(f, start_idx, end_idx, i).join();
    }

    return results;

}

vector<pair<int, vector<string> > > PropellerDetect::compute_candidates(vector<string> answer, vector<string> groundTruth,  int& numDetected, double& positionScore, Options opt){

    ofstream fw(opt.log_file);
    fw <<"POSITION:"<<endl;

    positionScore = 0.0;
    numDetected = 0;

    unordered_map<int, vector<string> > objectIdNicknames;
    vector<bool> visited(groundTruth.size(), false);
    vector<pair<int, vector<string> > > baskets;

    for(int aNum = 0; aNum < (int)answer.size(); aNum++)
    {
        fw <<answer[aNum];
        bool correctGuess = false;

        vector<string> tokens = Helper::split(answer[aNum], ',');
        if(tokens.size() != 7)
        {
            LOG(ERROR) << "Wrong number of tokens in line returned by test method." << endl;
            positionScore = -1.0;
            return baskets;
        }

        transform(tokens.begin(), tokens.end(), tokens.begin(), Helper::trim);

        string imageID = tokens[0];
        double line = stod(tokens[1]);
        double sample = stod(tokens[2]);
        int objectID = stoi(tokens[3]);

        if(objectID < 0 || objectID > 10000)
        {
            LOG(ERROR) << "Invalid objectID number: " << objectID << endl;
            positionScore = -1.0;
            return baskets;
        }

        for(int truth = 0; truth < (int)groundTruth.size(); truth++)
        {
            if(visited[truth])
                continue;

            vector<string> truthTokens = Helper::split(groundTruth.at(truth), ',');
            transform(truthTokens.begin(), truthTokens.end(), truthTokens.begin(), Helper::trim);

            if(Helper::iequals(truthTokens[TOKEN_IMAGE], imageID))

            {
                double tLine = stod(truthTokens[TOKEN_LINE]);
                double tSample = stod(truthTokens[TOKEN_SAMPLE]);

                double distance = sqrt((line - tLine) * (line - tLine) + (sample - tSample) * (sample - tSample));

                if(distance <= POSITION_TOLERANCE)
                {
                    numDetected++;
                    //Max score: 1,000,000
                    positionScore += (1000000.0 / groundTruth.size()) * (((double)numDetected) / (aNum + 1));

                    LOG(INFO)<<"Found '" << truthTokens[TOKEN_NICKNAME] << "' in " << imageID << "." << endl;
                    fw <<",true," + truthTokens[TOKEN_NICKNAME] + "\n";
                    correctGuess = true;

                    visited[truth] = true;

                    if(objectIdNicknames.find(objectID) == objectIdNicknames.end())
                        objectIdNicknames.insert(make_pair(objectID, vector<string>()));

                    objectIdNicknames.at(objectID).push_back(truthTokens[TOKEN_NICKNAME]);

                    //Put in basket
                    bool foundBasket = false;
                    for(auto && basket:baskets)
                    {
                        if(basket.first == objectID)
                        {
                            basket.second.push_back(truthTokens[TOKEN_NICKNAME]);

                            foundBasket = true;
                            break;
                        }
                    }

                    if(!foundBasket)
                    {
                        //New basket
                        baskets.push_back(make_pair(objectID, vector<string>()));
                        baskets.at(baskets.size() - 1).second.push_back(truthTokens[TOKEN_NICKNAME]);
                    }

                    //Can't match this answer again
                    break;
                }
            }
        }

        if(!correctGuess)
            fw << ",false\n";
    }

    return baskets;

}

double PropellerDetect::score_and_link(string candidates_from, Options opt){

    vector<pair<int, vector<string> > > baskets;
    int groundTruth_size;
    int answer_size;
    int numDetected;
    double positionScore;

    ScoreResult scores;
    ifstream in(candidates_from);
    cereal::BinaryInputArchive iarchive(in);
    //iarchive(baskets);
    iarchive(scores);

    answer_size = scores.answer_size;
    baskets = scores.baskets;
    groundTruth_size = scores.groundTruth_size;
    numDetected = scores.numDetected ;
    positionScore = scores.positionScore;

    ofstream fw(opt.log_file, std::fstream::app);

    fw << "\nLINKING:\n";

    double linkingScore = 0.0;

    for(auto basket:baskets)
    {
        if(basket.second.size() <= 1) //No potential to score
            continue;


        unordered_map<string, int> nickCounts;
        int maxCount = 0;
        string maxNick;

        for(string nick:basket.second)
        {
            if(nick.empty())
                continue;

            int count = 1;
            if(nickCounts.find(nick) != nickCounts.end())
            {
                count = nickCounts.at(nick) + 1;
                nickCounts[nick] = count;
            }
            else
            {
                nickCounts.insert(make_pair(nick, 1));
            }
            if(count > maxCount)
            {
                maxCount = count;
                maxNick = nick;
            }
        }

        if(maxCount <= 1) //No potential to score
            continue;

        LOG(INFO) << "Linked '" << maxNick << "' " << maxCount << " times."<<endl;

        fw << maxNick << "," << maxCount << "\n";

        //Max score: 200,000
        linkingScore += ((200000.0 / groundTruth_size) * (maxCount - 1) * (maxCount - 1)) / (basket.second.size() - 1);
    }

    fw << "\nSCORING:\n";

    LOG(INFO)<<"Correct detections: " << numDetected << " / " << groundTruth_size << " - " << (answer_size - numDetected) << " false positives." << endl;

    LOG(INFO) << "positionScore: " << positionScore << endl;
    LOG(INFO) << "linkingScore: " << linkingScore << endl;

    fw << "positionScore: " << std::setprecision(8) << positionScore << "\n";
    fw << "linkingScore: " << std::setprecision(8) << linkingScore << "\n";
    fw << "correct detections: " << numDetected << " / " << groundTruth_size << "\n";
    fw << "false positives: " << (answer_size - numDetected) << "\n";

    fw.close();

    return positionScore + linkingScore;

}

vector<string> PropellerDetect::loadGroundTruth(string fileName){

    vector<string> groundTruthLines;
    groundTruthLines = Helper::readTextFile(fileName);


    //Remove first line
    return vector<string>(next(groundTruthLines.begin(), 1), groundTruthLines.end() );

}

void PropellerDetect::test(string testLabel, string groundTruth, string model_from, string save_candidates_to, Options opt){

    //TESTING PHASE

    PropellerDetector prop; // load from file

    RandomForest rf; // load from file

    ifstream in(model_from);
    cereal::BinaryInputArchive iarchive(in);
    iarchive(rf, prop, currentImageId);

    LOG(INFO) << "Starting testing phase." << endl;

    vector<string> testingGroundTruth;
    {
        //Load local data to send for training
        testingGroundTruth = loadGroundTruth(groundTruth);

        //Load label file
        vector<PdsImageParameters> testingImageParamsList = readLabelFile(testLabel, opt);

        int start_idx = max(opt.start_index, 0);

        int end_idx = (opt.end_index < 0 ? testingImageParamsList.size() : min((int)testingImageParamsList.size(), opt.end_index+1) );

        //Re-set ground truth
        {
            vector<string> copyGroundTruth(testingGroundTruth);
            testingGroundTruth.clear();

            for(string gt:copyGroundTruth)
            {
                bool keep = false;

                for(int idx = start_idx; idx < end_idx; idx++)
                {
                    PdsImageParameters &imageParams = testingImageParamsList[idx];

                    string imageId = Helper::replace(Helper::replace(imageParams.getImageFilePath().substr(imageParams.getImageFilePath().find_last_of("/") + 1), "_CALIB", ""), ".IMG", "");
                    imageId = imageId.substr(0, imageId.size() - 2);

                    vector<string> tmp = Helper::split(gt, ',');
                    if(Helper::iequals(Helper::trim(tmp[TOKEN_IMAGE]), imageId))
                    {
                        keep = true;
                        break;
                    }

                }

                if(keep)
                {
                    testingGroundTruth.push_back(gt);
                }
            }
        }


        //Execute testing

		const int num_threads = opt.num_threads;
        vector< thread > threads;

		for(int thr = 0; thr < num_threads; thr++)
		{
			int s_index = start_idx + (end_idx - start_idx) * thr / num_threads;
			int e_index = start_idx + (end_idx - start_idx) * (thr + 1) / num_threads;

			threads.push_back(thread([&prop, &testingImageParamsList, start_idx](int start_index, int end_index) {

				for(int idx=start_index; idx < end_index; idx++)
				{
					PdsImageParameters &imageParams = testingImageParamsList[idx];
					string imageFilePath = imageParams.getImageFilePath();

					LOG(INFO) << "Reading testing image file (" << idx+1 << " / " << testingImageParamsList.size()<< ") " << imageFilePath << endl;

					PdsImageReader reader(imageFilePath);
					vector<vector<double> > imageData2D = reader.getImageData();
					vector<double> imageData(imageData2D.size() * imageData2D[0].size());

					if(imageData2D.size() != 1024 || imageData2D[0].size() != 1024)
					{
						LOG(ERROR) << "Only 1024x1024 images are supported. Image res: " << imageData2D.size() << "x" << imageData2D[0].size() << endl;
						continue;
					}

					int index = 0;
					for(int i = 0; i < imageData2D.size(); i++)
					{
						for(int j = 0; j < imageData2D[i].size(); j++)
						{
							imageData[index++] = imageData2D[i][j];
						}
					}


					string imageId = Helper::replace(Helper::replace(imageParams.getImageFilePath().substr(imageParams.getImageFilePath().find_last_of("/") + 1), "_CALIB", ""), ".IMG", "");
					imageId = imageId.substr(0, imageId.size() - 2);

					vector<double> image = imageData;
					vector<double> pos = imageParams.getScPlanetPositionVector();

					prop.testingData(image, imageId, imageParams.getStartTime(), imageParams.getDeclination(),
									 imageParams.getRightAscension(), imageParams.getTwistAngle(),
									 pos, imageParams.getInstrumentId(),
									 imageParams.getInstrumentModeId(), currentImageId + idx - start_idx);

				}
			}, s_index, e_index));
		}

		for(int thr = 0; thr < num_threads; thr++)
        {
			threads[thr].join();
        }
    }

    /*ofstream ofs("test_data.txt");
    for(int i = 0; i < prop.testData.size(); i++)
    {
		for(int j = 0; j < prop.testData[0].size(); j++)
		{
			ofs << prop.testData[i][j];
			if(j != prop.testData[0].size() - 1)
				ofs << ",";
		}
		ofs << endl;
    }
    ofs.close();*/

	LOG(INFO) << "Testing phase - RF training." << endl;
    //Get answers
    vector<string> answer =  prop.test(rf, opt);

    LOG(INFO) << "Testing phase - RF training complete." << endl;

    ofstream ofs("answer.txt");
    for(int i = 0; i < answer.size(); i++)
    {
		ofs << answer[i] << endl;
    }
    ofs.close();

    int numDetected = 0;
    double  positionScore = 0.0;
    vector<pair<int, vector<string> > > baskets = compute_candidates(answer, testingGroundTruth, numDetected,  positionScore, opt);

	LOG(INFO) << "Testing phase - computing candidates complete." << endl;

    // Save answer.size, testingGroundTruth.size, baskets, numDetected
    ofstream out(save_candidates_to);
    cereal::BinaryOutputArchive oarchive(out);
    ScoreResult scores;
    scores.answer_size = answer.size();
    scores.baskets = baskets;
    scores.groundTruth_size = testingGroundTruth.size();
    scores.numDetected =  numDetected;
    scores.positionScore = positionScore;
    oarchive(scores);

    LOG(INFO) << "Testing phase complete." << endl;
}

void PropellerDetect::train(string trainLabel, string groundTruth, string save_model_to, Options opt){

    //TRAINING PHASE
    PropellerDetector prop;

    LOG(INFO) << "Starting training phase." << endl;

    {
        //Load local data to send for training
        LOG(INFO) << "Loading ground truth from "<< groundTruth << endl;
        vector<string> localGroundTruth = loadGroundTruth(groundTruth);

        //Load label file
        vector<PdsImageParameters> trainingImageParamsList = readLabelFile(trainLabel, opt);

        int numTrain  = trainingImageParamsList.size();

        int start_idx = max(opt.start_index, 0);

        int end_idx = (opt.end_index < 0 ? numTrain : min((int)numTrain, opt.end_index+1) );

        vector<double> image;
        image.resize(1024 * 1024);
        //Execute training
        LOG(INFO)<< "Starting training"<<endl;

        const int num_threads = opt.num_threads;
        vector< thread > threads;

		for(int thr = 0; thr < num_threads; thr++)
		{
			int s_index = start_idx + (end_idx - start_idx) * thr / num_threads;
			int e_index = start_idx + (end_idx - start_idx) * (thr + 1) / num_threads;

			threads.push_back(thread([&prop, &localGroundTruth, &trainingImageParamsList, numTrain, start_idx](int start_index, int end_index) {

				for(int idx=start_index; idx < end_index; idx++)
				{
					PdsImageParameters &imageParams = trainingImageParamsList[idx];

					string imageFilePath =  imageParams.getImageFilePath();
					LOG(INFO) << "Reading training image file (" << idx+1 << " / " << numTrain << ") " << imageFilePath << endl;

					PdsImageReader reader(imageFilePath);
					vector<vector<double> > imageData2D = reader.getImageData();
					vector<double> imageData(imageData2D.size() * imageData2D[0].size(), 0);

					if(imageData2D.size() != 1024 || imageData2D[0].size() != 1024)
					{
						LOG(ERROR) << "Only 1024x1024 images are supported. Image res: " << imageData2D.size() << "x" << imageData2D[0].size() << endl;
						continue;
					}

					int index = 0;
					for(int i = 0; i < imageData2D.size(); i++)
					{
						for(int j = 0; j < imageData2D[i].size(); j++)
						{
							imageData[index++] = imageData2D[i][j];
						}
					}

					string imageId = Helper::replace(imageParams.getImageFilePath().substr(imageParams.getImageFilePath().find_last_of("/") + 1), "_CALIB.IMG", "");
					imageId = imageId.substr(0, imageId.size() - 2);

					vector<string> localTruth;

					for(string truth:localGroundTruth)
					{
						vector<string> tmp = Helper::split(truth, ',');
						if(Helper::iequals(Helper::trim(tmp[TOKEN_IMAGE]), imageId))
						{
							localTruth.push_back(truth);
						}
					}

					vector<double> image = imageData;
					vector<double> pos = imageParams.getScPlanetPositionVector();

					int res = prop.trainingData(image, imageId, imageParams.getStartTime(), imageParams.getDeclination(),
													  imageParams.getRightAscension(), imageParams.getTwistAngle(),
													  pos, imageParams.getInstrumentId(),
													  imageParams.getInstrumentModeId(), localTruth, idx - start_idx);

					//if (res == 1)
					//    break;
					//If they return 1, end training
				}
			}, s_index, e_index));
        }

        for(int thr = 0; thr < num_threads; thr++)
        {
			threads[thr].join();
        }
    }

     RandomForest rf = prop.train(opt);
     // Save the output and the Detector
     ofstream out(save_model_to);
     cereal::BinaryOutputArchive oarchive(out);
     oarchive(rf, prop, currentImageId);

     LOG(INFO) << "Training phase complete." << endl;
}

PropellerDetect::~PropellerDetect()
{

}

