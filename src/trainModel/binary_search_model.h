#ifndef BINARY_SEARCH_MODEL_H
#define BINARY_SEARCH_MODEL_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "../params.h"
using namespace std;
class BinarySearchModel
{
public:
    BinarySearchModel(){};
    BinarySearchModel(int childNum)
    {
        childNumber = childNum;
        for (int i = 0; i < childNum; i++)
            index.push_back(0);
    }
    void Train(const vector<pair<double, double>> &dataset, int len);
    int Predict(double key)
    {

        int start_idx = 0;
        int end_idx = childNumber - 1;
        int mid;
        while (start_idx < end_idx)
        {
            mid = (start_idx + end_idx) / 2;
            if (index[mid] < key)
                start_idx = mid + 1;
            else
                end_idx = mid;
        }
        return end_idx;
    }

    // designed for test
    void GetIndex(vector<double> &v)
    {
        for (int i = 0; i < childNumber; i++)
            v.push_back(index[i]);
    }
    int GetChildNum() { return childNumber; }

private:
    vector<double> index;  // 8c
    int childNumber;  // 4
};

void BinarySearchModel::Train(const vector<pair<double, double>> &dataset, int len)
{
    if (dataset.size() == 0)
        return;
    index.clear();
    float value = float(dataset.size()) / childNumber;
    int cnt = 1;
    for (int i = value * cnt - 1; i < dataset.size(); i = value * (++cnt) - 1)
    {
        if (dataset[i].first != -1)
        {
            index.push_back(dataset[i].first);
        }
        else
        {
            for (int j = i + 1; j < dataset.size(); j++)
            {
                if (dataset[j].first != -1)
                {
                    index.push_back(dataset[j].first);
                    break;
                }
            }
        }
    }
}

#endif