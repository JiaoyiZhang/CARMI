#ifndef UNIFORM_DISTRIBUTION_H
#define UNIFORM_DISTRIBUTION_H

#include <algorithm>
#include <random>
#include <iostream>
#include <chrono>
#include <vector>
using namespace std;

class UniformDataset
{
public:
    UniformDataset(int total, double initRatio)
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

void UniformDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &trainFindQuery, vector<pair<double, double>> &trainInsertQuery, vector<pair<double, double>> &testInsertQuery)
{
    vector<pair<double, double>>().swap(initDataset);
    vector<pair<double, double>>().swap(trainFindQuery);
    vector<pair<double, double>>().swap(trainInsertQuery);
    vector<pair<double, double>>().swap(testInsertQuery);

    vector<pair<double, double>> insertDataset;
    int cnt = 0;
    if (initSize == 0)
    {
        int i = 0;
        for (; i <= 0.6 * totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
        for (; i < 0.9 * totalSize; i += 2)
        {
            initDataset.push_back({double(i), double(i) * 10});
            insertDataset.push_back({double(i + 1), double(i + 1) * 10});
        }
        for (; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
    }
    else if (num == -1)
    {
        for (int i = 0; i < totalSize; i++)
            initDataset.push_back({double(i), double(i) * 10});
    }
    else
    {
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

    cout << "uniform: init size:" << initDataset.size() << "\tFind size:" << trainFindQuery.size() << "\tWrite size:" << testInsertQuery.size() << endl;
}

#endif