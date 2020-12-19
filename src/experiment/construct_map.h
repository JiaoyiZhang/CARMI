#ifndef CONSTRUCT_MAP_H
#define CONSTRUCT_MAP_H

#include "../dataset/longitudes.h"
#include "../dataset/longlat.h"

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"

#include "./core_construct.h"

extern double kRate;
extern int kMaxKeyNum;

extern vector<pair<double, double>> dataset;
extern vector<pair<double, double>> insertDataset;
extern ofstream outRes;

void constructMap(double initRatio)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    cout << "construct map" << endl;
    outRes << "construct map" << endl;
    outRes << "initRatio," << initRatio << endl;
    LongitudesDataset longData = LongitudesDataset(initRatio);
    LonglatDataset latData = LonglatDataset(initRatio);
    vector<double> rate = {1000, 100, 50, 10, 1, 0.6, 0.1, 0.01, 0.001};
    for (int r = 0; r < rate.size(); r++)
    {
        kRate = rate[r];
        outRes << "kRate:" << kRate << endl;

        kMaxKeyNum = 16;
        cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        latData.GenerateDataset(dataset, insertDataset);
        if (r == 0)
        {
            btree_test();
            artTree_test();
        }
        CoreConstruct();

        cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        longData.GenerateDataset(dataset, insertDataset);
        if (r == 0)
        {
            btree_test();
            artTree_test();
        }
        CoreConstruct();

        outRes << endl;
    }
}

#endif // !CONSTRUCT_MAP_H