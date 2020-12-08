#ifndef GREEDY_H
#define GREEDY_H

#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../function.h"
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
            vector<int> perSize(c, 0);
            for (int i = 0; i < c; i++)
                perSize.push_back(0);

            switch (type)
            {
            case 0:
            {
                time = 8.1624;
                space = float(4 * c + sizeof(LRType)) / 1024 / 1024;
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
                time = 20.26894;
                space = float(4 * c + 192 + sizeof(NNType)) / 1024 / 1024;
                auto root = NNType(c);
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
                time = 19.6543;
                space = float(5 * c + sizeof(HisType)) / 1024 / 1024;
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
                time = 4 * log(c) / log(2);
                space = float(12 * c + sizeof(BSType)) / 1024 / 1024;
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