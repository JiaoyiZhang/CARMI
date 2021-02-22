#ifndef CHOOSE_ROOT_H
#define CHOOSE_ROOT_H

#include "../nodes/rootNode/bin_type.h"
#include "../nodes/rootNode/his_type.h"
#include "../nodes/rootNode/lr_type.h"
#include "../nodes/rootNode/plr_type.h"
#include "../carmi.h"
#include "root_struct.h"
#include <vector>
#include <float.h>
using namespace std;

template <typename TYPE>
void CARMI::RootAllocSize(vector<int> &perSize, const int c)
{
    TYPE root = TYPE(c);
    root.model.Train(initDataset, c);
    for (int i = 0; i < initDataset.size(); i++)
    {
        int p = root.model.Predict(initDataset[i].first);
        perSize[p]++;
    }
}

RootStruct CARMI::ChooseRoot()
{
    double OptimalValue = DBL_MAX;
    double time, space;
    int optimalChildNumber, optimalType;
    for (int c = 1024; c <= initDataset.size() * 10; c *= 2)
    {
        if (512 * c < initDataset.size())
            continue;
        for (int type = 0; type < 4; type++)
        {
            vector<int> perSize(c, 0);
            space = 64.0 * c / 1024 / 1024;

            switch (type)
            {
            case 0:
            {
                time = 12.7013;
                RootAllocSize<LRType>(perSize, c);
                break;
            }
            case 1:
            {
                time = 39.6429;
                RootAllocSize<PLRType>(perSize, c);
                break;
            }
            case 2:
            {
                time = 44.2824;
                RootAllocSize<HisType>(perSize, c);
                break;
            }
            case 3:
            {
                time = 10.9438 * log(c) / log(2);
                RootAllocSize<BSType>(perSize, c);
                break;
            }
            }

            long double entropy = 0.0;
            for (int i = 0; i < c; i++)
            {
                auto p = float(perSize[i]) / initDataset.size();
                if (p != 0)
                    entropy += p * (-log(p) / log(2));
            }

            double cost = (time + float(kRate * space)) / entropy;
            if (cost <= OptimalValue)
            {
                optimalChildNumber = c;
                optimalType = type;
                OptimalValue = cost;
            }
        }
    }
#ifdef DEBUG
    cout << "Best type is: " << optimalType << "\tbest childNumber: " << optimalChildNumber << "\tOptimal Value: " << OptimalValue << endl;
#endif
    return {optimalType, optimalChildNumber};
}

#endif // !GREEDY_H