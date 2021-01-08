#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H

#include <vector>
#include "../params.h"
#include "../dataManager/datapoint.h"
#include "../baseNode.h"
#include "array.h"
using namespace std;

extern pair<double, double> *entireData;
extern vector<pair<double, double>> findDatapoint;

inline void ArrayType::SetDataset(const vector<pair<double, double>> &dataset, int cap)
{
    if (m_left != -1)
        releaseMemory(m_left, m_capacity);
    m_capacity = cap;
    int size = dataset.size();
    flagNumber += size;
    while (size >= m_capacity)
        m_capacity *= kExpansionScale;

    m_capacity *= 2; // test
    if (m_capacity > 4096)
        m_capacity = 4096;

    m_left = allocateMemory(m_capacity);
    if (size == 0)
        return;

    if (size > 4096)
        cout << "Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    for (int i = m_left, j = 0; j < size; i++, j++)
        entireData[i] = dataset[j];

    Train(dataset);
    UpdateError(dataset);
}

inline void ArrayType::SetDataset(const int left, const int size, int cap)
{
    flagNumber += size;
    int right = left + size;
    vector<pair<double, double>> tmp;
    for (int j = left; j < right; j++)
        tmp.push_back(entireData[j]);

    SetDataset(tmp, cap);
}

inline int ArrayType::UpdateError(const vector<pair<double, double>> &dataset)
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    int size = flagNumber & 0x00FFFFFF;
    for (int i = 0; i < size; i++)
    {
        p = Predict(dataset[i].first);
        d = abs(i - p);
        if (d > maxError)
            maxError = d;
    }

    // find the optimal value of error
    int minRes = size * log(size) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = 0; i < size; i++)
        {
            p = Predict(dataset[i].first);
            d = abs(i - p);
            if (d <= e)
                cntBetween++;
            else
                cntOut++;
        }
        if (e != 0)
            res = cntBetween * log(e) / log(2) + cntOut * log(size) / log(2);
        else
            res = cntOut * log(size) / log(2);
        if (res < minRes)
        {
            minRes = res;
            error = e;
        }
    }
    return error;
}

inline void ArrayType::Train(const vector<pair<double, double>> &dataset)
{
    int actualSize = 0;
    vector<double> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
            actualSize++;
        index.push_back(double(i) / double(dataset.size()));
    }
    if (actualSize == 0)
        return;

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            t1 += dataset[i].first * dataset[i].first;
            t2 += dataset[i].first;
            t3 += dataset[i].first * index[i];
            t4 += index[i];
        }
    }
    theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
}

inline int ArrayType::Predict(double key)
{
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0xFFFFFF);
    int p = (theta1 * key + theta2) * size;
    if (p < 0)
        p = 0;
    else if (p >= size)
        p = size - 1;
    return p;
}

inline int ArrayType::UpdateError(const int start_idx, const int size)
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    for (int i = start_idx; i < start_idx + size; i++)
    {
        p = Predict(findDatapoint[i].first);
        d = abs(i - start_idx - p);
        if (d > maxError)
            maxError = d;
    }

    // find the optimal value of error
    int minRes = size * log(size) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = start_idx; i < start_idx + size; i++)
        {
            p = Predict(findDatapoint[i].first);
            d = abs(i - start_idx - p);
            if (d <= e)
                cntBetween++;
            else
                cntOut++;
        }
        if (e != 0)
            res = cntBetween * log(e) / log(2) + cntOut * log(size) / log(2);
        else
            res = cntOut * log(size) / log(2);
        if (res < minRes)
        {
            minRes = res;
            error = e;
        }
    }
    return error;
}

inline void ArrayType::Train(const int start_idx, const int size)
{
    vector<double> index;
    for (int i = start_idx; i < start_idx + size; i++)
        index.push_back(double(i - start_idx) / double(size));

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = start_idx; i < start_idx + size; i++)
    {
        t1 += findDatapoint[i].first * findDatapoint[i].first;
        t2 += findDatapoint[i].first;
        t3 += findDatapoint[i].first * index[i];
        t4 += index[i];
    }
    theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
}

#endif