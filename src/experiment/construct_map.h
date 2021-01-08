#ifndef CONSTRUCT_MAP_H
#define CONSTRUCT_MAP_H

#include "../dataset/longitudes.h"
#include "../dataset/longlat.h"

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"

#include "./core_construct.h"

extern double kRate;

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
    // vector<double> rate = {5, 1, 0.8, 0.6, 0.4, 0.3, 0.2, 0.1};
    vector<double> rate = {5, 1, 0.8, 0.6, 0.4, 0.2};
    for (int r = 0; r < rate.size(); r++)
    {
        kRate = rate[r];
        outRes << "kRate:" << kRate << endl;

        kMaxKeyNum = 16;
        cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++" << endl;
        latData.GenerateDataset(dataset, insertDataset);
        if (r == 0)
        {
            btree_test(initRatio);
            // artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++" << endl;
        longData.GenerateDataset(dataset, insertDataset);
        if (r == 0)
        {
            btree_test(initRatio);
            // artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        outRes << endl;
    }
}

#endif // !CONSTRUCT_MAP_H