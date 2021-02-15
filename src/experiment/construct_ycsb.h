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
extern vector<int> length;

void constructYCSB(double initRatio)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    cout << "initRatio is: " << initRatio << endl;
    cout << "construct ycsb" << endl;
    outRes << "construct ycsb" << endl;
    outRes << "initRatio," << initRatio << endl;
    kIsYCSB = true;
    double init = initRatio;
    if (init == 2)
        init = 0.95;
    YCSBDataset ycsbData = YCSBDataset(init);

    vector<double> rate = {0.4, 0.3, 0.25, 0.22, 0.2, 0.1};
    vector<double> rate1 = {0.6, 0.4, 0.3, 0.25, 0.2, 0.1}; // 0.5

    for (int r = 0; r < rate.size(); r++)
    // for (int r = 0; r < 1; r++)
    {
        if (initRatio == 0.5)
            kRate = rate1[r];
        else
            kRate = rate[r];
        cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++" << endl;
        outRes << "kRate:" << kRate << endl;
        ycsbData.GenerateDataset(dataset, insertDataset);
        if (initRatio == 2)
        {
            length.clear();
            srand(time(0));
            for (int i = 0; i < dataset.size(); i++)
            {
                length.push_back(rand() % 100 + 1);
            }
        }

        if (r == 0)
        {
            // btree_test(initRatio);
            // artTree_test(initRatio);
        }
        CoreConstruct(initRatio);

        outRes << endl;
    }
    kIsYCSB = false;
}

#endif // !CONSTRUCT_YCSB_H