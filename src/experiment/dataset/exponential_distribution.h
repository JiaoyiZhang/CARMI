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
    float maxValue = totalSize;

    // create dataset randomly
    std::default_random_engine generator;
    std::exponential_distribution<double> distribution(0.25);
    vector<double> ds;

    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(trainFindQuery);
    vector<pair<double, double>>().swap(trainInsertQuery);
    vector<pair<double, double>>().swap(testInsertQuery);

    vector<pair<double, double>> insertDataset;

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
            insertDataset.push_back({double(ds[i + 1] * factor), double(ds[i + 1] * factor) * 10});
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
    default_random_engine engine;

    auto find = initDataset;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed);
    shuffle(find.begin(), find.end(), engine);

    int end = 100000 - insertNumber;
    for (int i = 0; i < end; i++)
        trainFindQuery.push_back(initDataset[i]);

    unsigned seed1 = chrono::system_clock::now().time_since_epoch().count();
    engine = default_random_engine(seed1);
    shuffle(insertDataset.begin(), insertDataset.end(), engine);
    end = insertNumber * 2;
    for (int i = 0; i < end; i += 2)
    {
        trainInsertQuery.push_back(insertDataset[i]);
        testInsertQuery.push_back(insertDataset[i + 1]);
    }

    cout << "exponential: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif