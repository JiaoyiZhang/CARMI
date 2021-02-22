#ifndef FIXED_EXPERIMENT_H
#define FIXED_EXPERIMENT_H
#include "static.h"

#include "dataset/lognormal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/normal_distribution.h"
#include "dataset/exponential_distribution.h"

void fixedSynthetic(int datasetSize, double initRatio, vector<int> &length, int kLeafID);

void fixedExperiment(int datasetSize)
{
    // for range scan
    vector<int> length;
    // static structure
    for (int i = 0; i < 2; i++)
    {
        cout << "kleafnode:" << i << endl;
        fixedSynthetic(datasetSize, 1, length, i);
        fixedSynthetic(datasetSize, 0.5, length, i);
    }
    fixedSynthetic(datasetSize, 0.95, length, 0);
    fixedSynthetic(datasetSize, 0, length, 0);

    srand(time(0));
    for (int i = 0; i < datasetSize; i++)
    {
        length.push_back(min(i + rand() % 100 + 1, datasetSize) - i);
    }
    fixedSynthetic(datasetSize, 2, length, 0);
}

void fixedSynthetic(int datasetSize, double initRatio, vector<int> &length, int kLeafID)
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

    for (int i = 0; i < 1; i++)
    {
        int childNum = 131072;
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        uniData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        outRes << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio, initData, testInsert, length, kLeafID);
        break;

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        expData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio, initData, testInsert, length, kLeafID);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        norData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio, initData, testInsert, length, kLeafID);

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        logData.GenerateDataset(initData, trainFind, trainInsert, testInsert);
        outRes << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio, initData, testInsert, length, kLeafID);
    }
}

#endif // !FIXED_EXPERIMENT_H