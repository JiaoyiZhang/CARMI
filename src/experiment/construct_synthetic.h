#ifndef CONSTRUCT_SYNTHETIC_H
#define CONSTRUCT_SYNTHETIC_H

#include "dataset/lognormal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/normal_distribution.h"
#include "dataset/exponential_distribution.h"

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
extern vector<int> length;

void constructSynthetic(double initRatio)
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

    vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
    vector<double> rate1 = {
        // 0.6,
        0.4,
        0.3,
        0.25,
        0.2,
        0.1,
    }; // 0.5

    // for (int r = 0; r < rate.size(); r++)
    for (int r = 0; r < 1; r++)
    {
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        outRes << "kRate:" << kRate << endl;
        cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
        uniData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (initRatio == 2)
        {
            length.clear();
            srand(time(0));
            for (int i = 0; i < dataset.size(); i++)
            {
                length.push_back(min(i + rand() % 100 + 1, dataset.size()) - i);
            }
        }
        if (r == 0)
        {
            btree_test(initRatio);
            // artTree_test(initRatio);
        }
        // CoreConstruct(initRatio);

        // cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++" << endl;
        // expData.GenerateDataset(dataset, insertDataset);
        // initDatasetSize = dataset.size();
        // if (r == 0)
        // {
        //     // btree_test(initRatio);
        //     // artTree_test(initRatio);
        // }
        // CoreConstruct(initRatio);

        cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
        norData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (r == 0)
        {
            btree_test(initRatio);
            // artTree_test(initRatio);
        }
        // CoreConstruct(initRatio);

        cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
        logData.GenerateDataset(dataset, insertDataset);
        initDatasetSize = dataset.size();
        if (r == 0)
        {
            btree_test(initRatio);
            // artTree_test(initRatio);
        }
        // CoreConstruct(initRatio);

        outRes << endl;
    }
}

#endif // !CONSTRUCT_SYNTHETIC_H