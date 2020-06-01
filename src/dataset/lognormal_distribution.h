#ifndef LOGNORMAL_DISTRIBUTION_H
#define LOGNORMAL_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
using namespace std;

class LognormalDataset
{
public:
    LognormalDataset(int total, double initRatio)
    {
        totalSize = total;
        initSize = total * initRatio;
        insertSize = totalSize - initSize;

        // num = initRatio * 10;
        num = initRatio / (1 - initRatio);
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
    int totalSize;
    int insertSize;
    int initSize;

    int num;
};

void LognormalDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    float maxValue = totalSize;

    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);
    vector<double> ds;

    initDataset.clear();
    insertDataset.clear();

    for (int i = 0; i < totalSize; i++)
    {
        ds.push_back(distribution(generator));
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
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
    cout<<"Read size:"<<initDataset.size()<<"\tWrite size:"<<insertDataset.size()<<endl;
}

#endif