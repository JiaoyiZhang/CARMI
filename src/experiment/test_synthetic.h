#ifndef TEST_SYNTHETIC_H
#define TEST_SYNTHETIC_H

#include "../dataset/lognormal_distribution.h"
#include "../dataset/uniform_distribution.h"
#include "../dataset/normal_distribution.h"
#include "../dataset/exponential_distribution.h"

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"

#include "./static.h"

extern int datasetSize;
extern int initDatasetSize;
extern int childNum;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern ofstream outRes;

void testSynthetic(double initRatio)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);

    vector<int> childNum_synthetic = {2000, 3000, 3907, 5000, 7500, 10000, 12500, 15000, 17500, 20000, 50000, 100000};
    vector<int> childNum_map = {25000, 50000, 60000, 70125, 80000, 90000, 100000, 250000, 350000, 500000, 750000, 1000000};
    // for (int i = 0; i < childNum_synthetic.size(); i++)
    for (int i = 2; i < 3; i++)
    {
        childNum = childNum_synthetic[i];
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        uniData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        RunStatic();

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        expData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        RunStatic();

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        norData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        RunStatic();

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        logData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        RunStatic();
    }
}

#endif // !TEST_SYNTHETIC_H