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
    double init = initRatio;
    if (init == 2)
        init = 0.95;
    LognormalDataset logData = LognormalDataset(datasetSize, init);
    UniformDataset uniData = UniformDataset(datasetSize, init);
    NormalDataset norData = NormalDataset(datasetSize, init);
    ExponentialDataset expData = ExponentialDataset(datasetSize, init);

    vector<int> childNum_synthetic = {65536, 131072, 262144, 524288, 1048576};
    // for (int i = 0; i < childNum_synthetic.size(); i++)
    for (int i = 0; i < 1; i++)
    {
        // childNum = childNum_synthetic[i];
        childNum = 131072;
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        uniData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        // btree_test(initRatio);
        // artTree_test(initRatio);
        RunStatic(initRatio);
        // TestStatic();
        break;

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        expData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio);
        // btree_test(initRatio);
        // artTree_test(initRatio);
        // TestStatic();

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        norData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        RunStatic(initRatio);
        // btree_test(initRatio);
        // artTree_test(initRatio);
        // TestStatic();

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        outRes << "+++++++++++ childNum: " << childNum << endl;
        logData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        outRes << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        // btree_test(initRatio);
        // artTree_test(initRatio);
        // TestStatic();
        RunStatic(initRatio);
    }
}

#endif // !TEST_SYNTHETIC_H