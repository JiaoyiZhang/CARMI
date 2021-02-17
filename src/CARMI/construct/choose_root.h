#ifndef CHOOSE_ROOT_H
#define CHOOSE_ROOT_H

#include "../nodes/rootNode/bin_type.h"
#include "../nodes/rootNode/his_type.h"
#include "../nodes/rootNode/lr_type.h"
#include "../nodes/rootNode/plr_type.h"
#include "../func/function.h"
#include <vector>
using namespace std;

extern double kRate;

pair<int, int> ChooseRoot(const vector<pair<double, double>> &dataset)
{
    double OptimalValue = DBL_MAX;
    double time, space;
    int c;
    int optimalChildNumber, optimalType;
    vector<pair<double, double>> tmp;
    for (int c = 1024; c <= dataset.size() * 10; c *= 2)
    {
        if (512 * c < dataset.size())
            continue;
        for (int type = 0; type < 4; type++)
        {
            if (type == 1)
                continue;
            vector<int> perSize(c, 0);
            for (int i = 0; i < c; i++)
                perSize.push_back(0);

            switch (type)
            {
            case 0:
            {
                time = 12.7013;
                space = 64.0 * c / 1024 / 1024;
                auto root = LRType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSize[p]++;
                }
                break;
            }
            case 1:
            {
                time = 39.6429;
                space = 64.0 * c / 1024 / 1024;
                auto root = PLRType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSize[p]++;
                }
                break;
            }
            case 2:
            {
                time = 44.2824;
                space = 64.0 * c / 1024 / 1024;
                auto root = HisType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSize[p]++;
                }
                break;
            }
            case 3:
            {
                time = 10.9438 * log(c) / log(2);
                space = 64.0 * c / 1024 / 1024;
                auto root = BSType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSize[p]++;
                }
                break;
            }
            }

            long double entropy = 0.0;
            for (int i = 0; i < c; i++)
            {
                auto p = float(perSize[i]) / dataset.size();
                if (p != 0)
                    entropy += p * (-log(p) / log(2));
            }
            // entropy /= (log(c) / log(2));

            double cost = (time + float(kRate * space)) / entropy;
            if (cost <= OptimalValue)
            {
                optimalChildNumber = c;
                optimalType = type;
                OptimalValue = cost;
            }
        }
    }
    cout << "Best type is: " << optimalType << "\tbest childNumber: " << optimalChildNumber << "\tOptimal Value: " << OptimalValue << endl;
    return {optimalType, optimalChildNumber};
}

#endif // !GREEDY_H