#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
using namespace std;

class UniformDataset
{
public:
    UniformDataset(int total, double initRatio)
    {
        totalSize = total;
        initSize = total * initRatio;
        insertSize = totalSize - initSize;

        num = initRatio * 10;
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
    int totalSize;
    int insertSize;
    int initSize;

    int num;
};

void UniformDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
    // create dataset randomly
    // std::default_random_engine generator;
    // std::uniform_real_distribution<double> distribution(0.0, double(totalSize));
    // vector<double> ds;

    // for (int i = 0; i < totalSize; i++)
    // {
    //     ds.push_back(distribution(generator));
    // }
    // std::sort(ds.begin(), ds.end());

    int cnt = 0;
    for (int i = 0; i < totalSize; i++)
    {
        cnt++;
        if (cnt <= num)
        {
            initDataset.push_back({double(i), double(i) * 10});
        }
        else
        {
            insertDataset.push_back({double(i), double(i) * 10});
            if (cnt == 10)
                cnt = 0;
        }
    }
}

#endif