#ifndef CONSTRUCT_YCSB_H
#define CONSTRUCT_YCSB_H

#include "../dataset/ycsb.h"
#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"
#include "./core_construct.h"

extern double kRate;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern ofstream outRes;

void constructYCSB(double initRatio)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    cout << "construct ycsb" << endl;
    outRes << "construct ycsb" << endl;
    outRes << "initRatio," << initRatio << endl;
    YCSBDataset ycsbData = YCSBDataset(initRatio);
    vector<double> rate = {1000, 100, 50, 10, 1, 0.6, 0.1, 0.01, 0.001};
    for (int r = 0; r < rate.size(); r++)
    {
        kRate = rate[r];
        outRes << "kRate:" << kRate << endl;

        cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++" << endl;
        ycsbData.GenerateDataset(dataset, insertDataset);
        if (r == 0)
        {
            btree_test(initRatio);
            artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        outRes << endl;
    }
}

#endif // !CONSTRUCT_YCSB_H