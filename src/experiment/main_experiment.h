#ifndef MAIN_EXPERIMENT_H
#define MAIN_EXPERIMENT_H

#include "dataset/lognormal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/normal_distribution.h"
#include "dataset/exponential_distribution.h"

#include "dataset/longitudes.h"
#include "dataset/longlat.h"

#include "dataset/ycsb.h"

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"

#include "core.h"
extern ofstream outRes;

void mainSynthetic(int datasetSize, double initRatio, vector<int> &length, int thre);
void mainYCSB(double initRatio, vector<int> &length, int thre);
void mainMap(double initRatio, vector<int> &length, int thre);

void mainExperiment(int datasetSize, int thre)
{
    // for range scan
    vector<int> length;

    // read-only
    mainSynthetic(datasetSize, 1, length, thre);
    mainYCSB(1, length, thre);
    mainMap(1, length, thre);

    // write-heavy
    mainSynthetic(datasetSize, 0.5, length, thre);
    mainYCSB(0.5, length, thre);
    mainMap(0.5, length, thre);

    // read-heavy
    mainSynthetic(datasetSize, 0.95, length, thre);
    mainYCSB(0.95, length, thre);
    mainMap(0.95, length, thre);

    // write-partial
    mainSynthetic(datasetSize, 0, length, thre);
    mainYCSB(0, length, thre);
    mainMap(0, length, thre);

    // range scan
    srand(time(0));
    for (int i = 0; i < datasetSize; i++)
    {
        length.push_back(min(i + rand() % 100 + 1, datasetSize) - i);
    }
    mainSynthetic(datasetSize, 2, length, thre);
    mainYCSB(2, length, thre);
    mainMap(2, length, thre);
}

void mainSynthetic(int datasetSize, double initRatio, vector<int> &length, int thre)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    double init = initRatio;
    if (init == 2)
        init = 0.95;
    LognormalDataset logData = LognormalDataset(datasetSize, init);
    UniformDataset uniData = UniformDataset(datasetSize, init);
    NormalDataset norData = NormalDataset(datasetSize, init);
    ExponentialDataset expData = ExponentialDataset(datasetSize, init);

    vector<pair<double, double>> initData;
    vector<pair<double, double>> trainFind;
    vector<pair<double, double>> trainInsert;
    vector<pair<double, double>> testInsert;

    vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
    vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05}; // 0.5

    // for (int r = 0; r < rate.size(); r++)
    for (int r = 3; r < 4; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        // cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        // uniData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        // if (r == 3)
        // {
        //     // btree_test(initRatio, initData, testInsert, length);
        //     // artTree_test(initRatio, initData, testInsert, length);
        // }
        // Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        expData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        norData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        logData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        outRes << endl;
    }
}

void mainMap(double initRatio, vector<int> &length, int thre)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    cout << "construct map" << endl;
    outRes << "construct map" << endl;
    cout << "kAlgThre:" << thre << endl;
    double init = initRatio;
    if (init == 2)
        init = 0.95;
    LongitudesDataset longData = LongitudesDataset(init);
    LonglatDataset latData = LonglatDataset(init);

    vector<pair<double, double>> initData;
    vector<pair<double, double>> trainFind;
    vector<pair<double, double>> trainInsert;
    vector<pair<double, double>> testInsert;

    vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
    vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05}; // 0.5

    // for (int r = 0; r < rate.size(); r++)
    for (int r = 3; r < 4; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        latData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        longData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        outRes << endl;
    }
}

void mainYCSB(double initRatio, vector<int> &length, int thre)
{
    kPrimaryIndex = true;
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    cout << "construct ycsb" << endl;
    outRes << "construct ycsb" << endl;
    double init = initRatio;
    if (init == 2)
        init = 0.95;
    YCSBDataset ycsbData = YCSBDataset(init);

    vector<pair<double, double>> initData;
    vector<pair<double, double>> trainFind;
    vector<pair<double, double>> trainInsert;
    vector<pair<double, double>> testInsert;

    vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
    vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05}; // 0.5
    // for (int r = 0; r < rate.size(); r++)
    for (int r = 4; r < 5; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++" << endl;
        ycsbData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 3)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate, thre);

        outRes << endl;
    }
    kPrimaryIndex = false;
}

#endif // !MAIN_EXPERIMENT_H