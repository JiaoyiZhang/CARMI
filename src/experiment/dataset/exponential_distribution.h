#ifndef EXPONENTIAL_DISTRIBUTION_H
#define EXPONENTIAL_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

class ExponentialDataset
{
public:
    ExponentialDataset(int total, double initRatio)
    {
        num = 2;
        totalSize = total / initRatio;
        insertNumber = 100000 * (1 - initRatio);
        if (initRatio == 0)
        { // several leaf nodes are inserted
            initSize = 0;
            totalSize = total / 0.85 + 1;
            insertNumber = 15000;
        }
        else if (initRatio == 1)
        {
            num = -1;
            initSize = total;
        }
        else
        {
            initSize = total;
            num = round(initRatio / (1 - initRatio));
        }
    }

    void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery);

private:
    int totalSize;
    int initSize;

    int num;
    int insertNumber;
};

void ExponentialDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
    if (initSize != 0 && num != -1)
        totalSize *= 1.5;
    float maxValue = totalSize;

    // create dataset randomly
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.25);
    vector<double> ds;

    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(trainFindQuery);
    vector<pair<double, double>>().swap(trainInsertQuery);
    vector<pair<double, double>>().swap(testInsertQuery);

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
        for (; i <= 0.6 * totalSize; i++)
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
        for (; i < 0.9 * totalSize; i += 2)
        {
            initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            trainInsertQuery.push_back({double(ds[i + 1] * factor), double(ds[i + 1] * factor) * 10});
            if (testInsertQuery.size() < insertNumber)
                testInsertQuery.push_back({double((ds[i + 1] + 0.0001) * factor), double((ds[i + 1] + 0.0001) * factor) * 10});
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
                if (initDataset.size() == 67108864)
                    break;
                initDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            }
            else
            {
                trainInsertQuery.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
                i++;
                if (testInsertQuery.size() < insertNumber)
                    testInsertQuery.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
                cnt = 0;
            }
        }
    }
    trainFindQuery = initDataset;

    cout << "exponential: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\ttrain insert size:" << trainInsertQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif