#ifndef CONSTRUCT_SYNTHETIC_H
#define CONSTRUCT_SYNTHETIC_H

#include "../dataset/lognormal_distribution.h"
#include "../dataset/uniform_distribution.h"
#include "../dataset/normal_distribution.h"
#include "../dataset/exponential_distribution.h"

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"

#include "./core_construct.h"

extern double kRate;
extern int datasetSize;
extern int initDatasetSize;
extern int childNum;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern ofstream outRes;

void constructSynthetic(double initRatio)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    outRes << "initRatio," << initRatio << endl;
    LognormalDataset logData = LognormalDataset(datasetSize, initRatio);
    UniformDataset uniData = UniformDataset(datasetSize, initRatio);
    NormalDataset norData = NormalDataset(datasetSize, initRatio);
    ExponentialDataset expData = ExponentialDataset(datasetSize, initRatio);

    vector<double> rate = {50, 30, 25, 20, 10, 5, 1};
    for (int r = 0; r < rate.size(); r++)
    {
        kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        uniData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (r == 0)
        {
            btree_test(initRatio);
            artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        expData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (r == 0)
        {
            btree_test(initRatio);
            artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        norData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (r == 0)
        {
            btree_test(initRatio);
            artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        // cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        // logData.GenerateDataset(dataset, insertDataset);
        // initDatasetSize = dataset.size();
        // if (r == 0)
        // {
        //     btree_test(initRatio);
        //     artTree_test(initRatio);
        // }
        // CoreConstruct(initRatio);

        outRes << endl;
    }
}

#endif // !CONSTRUCT_SYNTHETIC_H