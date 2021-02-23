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

void mainSynthetic(int datasetSize, double initRatio, vector<int> &length);
void mainYCSB(double initRatio, vector<int> &length);
void mainMap(double initRatio, vector<int> &length);

void mainExperiment(int datasetSize)
{
    // for range scan
    vector<int> length;

    // read-only
    mainSynthetic(datasetSize, 1, length);
    mainYCSB(1, length);
    mainMap(1, length);

    // write-heavy
    mainSynthetic(datasetSize, 0.5, length);
    mainYCSB(0.5, length);
    mainMap(0.5, length);

    // read-heavy
    mainSynthetic(datasetSize, 0.95, length);
    mainYCSB(0.95, length);
    mainMap(0.95, length);

    // write-partial
    mainSynthetic(datasetSize, 0, length);
    mainYCSB(0, length);
    mainMap(0, length);

    // range scan
    srand(time(0));
    for (int i = 0; i < datasetSize; i++)
    {
        length.push_back(min(i + rand() % 100 + 1, datasetSize) - i);
    }
    mainSynthetic(datasetSize, 2, length);
    mainYCSB(2, length);
    mainMap(2, length);
}

void mainSynthetic(int datasetSize, double initRatio, vector<int> &length)
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
    vector<double> rate1 = {0.4, 0.3, 0.25, 0.2, 0.1}; // 0.5

    // for (int r = 0; r < rate.size(); r++)
    for (int r = 0; r < 1; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        uniData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        expData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        norData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        logData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        outRes << endl;
    }
}

void mainMap(double initRatio, vector<int> &length)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    cout << "construct map" << endl;
    outRes << "construct map" << endl;
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
    vector<double> rate1 = {0.4, 0.3, 0.25, 0.2, 0.1}; // 0.5

    // for (int r = 0; r < rate.size(); r++)
    for (int r = 0; r < 1; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        latData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        longData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        outRes << endl;
    }
}

void mainYCSB(double initRatio, vector<int> &length)
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
    vector<double> rate1 = {0.4, 0.3, 0.25, 0.2, 0.1}; // 0.5
    // for (int r = 0; r < rate.size(); r++)
    for (int r = 0; r < 1; r++)
    {
        double kRate;
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++" << endl;
        ycsbData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        if (r == 0)
        {
            // btree_test(initRatio, initData, testInsert, length);
            // artTree_test(initRatio, initData, testInsert, length);
        }
        Core(initRatio, initData, trainFind, trainInsert, testInsert, length, kRate);

        outRes << endl;
    }
    kPrimaryIndex = false;
}

#endif // !MAIN_EXPERIMENT_H