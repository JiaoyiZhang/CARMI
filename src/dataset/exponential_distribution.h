#ifndef EXPONENTIAL_DISTRIBUTION_H
#define EXPONENTIAL_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
using namespace std;

class ExponentialDataset
{
public:
    ExponentialDataset(int total, double initRatio)
    {
        totalSize = total;
        if (initRatio == 0)
        { // several leaf nodes are inserted
            insertSize = 0;
            initSize = 0;
        }
        else if (initRatio == 1)
        {
            num = -1;
            initSize = total;
            insertSize = 0;
        }
        else
        {
            initSize = total * initRatio;
            insertSize = totalSize - initSize;
            num = initRatio / (1 - initRatio);
        }
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
    int totalSize;
    int insertSize;
    int initSize;

    int num;
};

void ExponentialDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    float maxValue = totalSize;

    // create dataset randomly
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.25);
    vector<double> ds;

    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(insertDataset);

    for (int i = 0; i < totalSize; i++)
    {
        ds.push_back(distribution(generator));
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
    if (initSize == 0)
    {
        int i = 0;
        for (; i < 0.6 * totalSize; i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
        for (; i < 0.9 * totalSize; i += 2)
        {
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            insertDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
        }
        for (; i < totalSize; i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    else if (num == -1)
    {
        for (int i = 0; i < ds.size(); i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    else
    {
        int cnt = 0;
        for (int i = 0; i < ds.size(); i++)
        {
            cnt++;
            if (cnt <= num)
            {
                initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            }
            else
            {
                insertDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
                cnt = 0;
            }
        }
    }
    cout << "exponential: Read size:" << initDataset.size() << "\tWrite size:" << insertDataset.size() << endl;
}

#endif