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
    for (int c = 1024; c <= dataset.size();)
    {
        // if (c <= 4096)
        //     c *= 2;
        // else if (c <= 40960)
        //     c += 8192;
        // else if (c <= 1000000)
        //     c += 65536;
        // else
            c *= 2;
        if (512 * c < dataset.size())
            continue;
        for (int type = 0; type < 4; type++)
        {
            // cout << "now childNumber:" << c << ",\ttype:" << type << endl;
            vector<vector<pair<double, double>>> perSubDataset;
            for (int i = 0; i < c; i++)
                perSubDataset.push_back(tmp);

            switch (type)
            {
            case 0:
            {
                time = 8.1624;
                space = 4 * c + sizeof(LRType);
                auto root = LRType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSubDataset[p].push_back(dataset[i]);
                }
                break;
            }
            case 1:
            {
                time = 20.26894;
                space = 4 * c + 192 + sizeof(NNType);
                auto root = NNType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSubDataset[p].push_back(dataset[i]);
                }
                break;
            }
            case 2:
            {
                time = 19.6543;
                space = 5 * c + sizeof(HisType);
                auto root = HisType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSubDataset[p].push_back(dataset[i]);
                }
                break;
            }
            case 3:
            {
                time = 4 * log(c) / log(2);
                space = 12 * c + sizeof(BSType);
                auto root = BSType(c);
                root.model.Train(dataset, c);
                for (int i = 0; i < dataset.size(); i++)
                {
                    int p = root.model.Predict(dataset[i].first);
                    perSubDataset[p].push_back(dataset[i]);
                }
                break;
            }
            }

            long double entropy = 0.0;
            for (int i = 0; i < c; i++)
            {
                auto p = float(perSubDataset[i].size()) / dataset.size();
                if (p != 0)
                    entropy += p * (-log(p) / log(2));
            }
            // entropy /= (log(c) / log(2));
            vector<vector<pair<double, double>>>().swap(perSubDataset);

            double cost = (time + (float(kRate * space) / dataset.size())) / entropy;  // ns / data + ns/B * B / data
            // cout << "time:" << time << ",\tspace:" << space << ",\tavg space:" << float(space) / float(dataset.size()) << ",\tk space:" << float(kRate * space) / float(dataset.size()) << endl;
            // cout << "entropy:" << entropy << endl;
            // cout << "ratio:" << cost << endl;
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