#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H

#include <vector>
#include "../params.h"
#include "../dataManager/datapoint.h"
#include "../baseNode.h"
#include "array.h"
using namespace std;

extern pair<double, double> *entireData;
extern vector<pair<double, double>> findActualDataset;

inline void ArrayType::SetDataset(const vector<pair<double, double>> &dataset, int cap)
{
    if (m_left != -1)
        releaseMemory(m_left, m_capacity);
    m_capacity = cap;
    int size = dataset.size();
    flagNumber += size;
    while (size > m_capacity)
        m_capacity *= kExpansionScale;

    // m_capacity *= 2; // test
    if (m_capacity > 4096)
        m_capacity = 4096;

    m_left = allocateMemory(m_capacity);
    if (size == 0)
        return;

    if (size > 4096)
        cout << "Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    for (int i = m_left, j = 0; j < size; i++, j++)
        entireData[i] = dataset[j];

    Train();
    UpdateError();
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

inline int ArrayType::UpdateError()
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    int size = flagNumber & 0x00FFFFFF;
    int end = m_left + size;
    for (int i = m_left; i < end; i++)
    {
        p = Predict(entireData[i].first) + m_left;
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
        for (int i = m_left; i < end; i++)
        {
            p = Predict(entireData[i].first) + m_left;
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

inline void ArrayType::Train()
{
    int actualSize = 0;
    vector<double> index;
    int size = flagNumber & 0x00FFFFFF;
    int end = m_left + size;
    for (int i = m_left; i < end; i++)
    {
        if (entireData[i].first != DBL_MIN)
            actualSize++;
        index.push_back(double(i - m_left) / double(size));
    }
    if (actualSize == 0)
        return;

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = m_left; i < end; i++)
    {
        if (entireData[i].first != DBL_MIN)
        {
            t1 += entireData[i].first * entireData[i].first;
            t2 += entireData[i].first;
            t3 += entireData[i].first * index[i - m_left];
            t4 += index[i - m_left];
        }
    }
    theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
}

inline int ArrayType::Predict(double key)
{
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0x00FFFFFF);
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
        p = Predict(findActualDataset[i].first);
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
            p = Predict(findActualDataset[i].first);
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
    if ((flagNumber & 0x00FFFFFF) != size)
        flagNumber += size;
    vector<double> index;
    int end = start_idx + size;
    for (int i = start_idx; i < end; i++)
        index.push_back(double(i - start_idx) / double(size));

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = start_idx; i < end; i++)
    {
        t1 += findActualDataset[i].first * findActualDataset[i].first;
        t2 += findActualDataset[i].first;
        t3 += findActualDataset[i].first * index[i - start_idx];
        t4 += index[i - start_idx];
    }
    theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
}

inline void ArrayType::SetDataset(const int start_idx, const int size)
{
    if (m_left != -1)
        releaseMemory(m_left, m_capacity);
    flagNumber += size;
    while (size > m_capacity)
        m_capacity *= kExpansionScale;

    // m_capacity *= 2; // test
    if (m_capacity > 4096)
        m_capacity = 4096;

    m_left = allocateMemory(m_capacity);
    if (size == 0)
        return;

    if (size > 4096)
        cout << "Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    int end = start_idx + size;
    for (int i = m_left, j = start_idx; j < end; i++, j++)
        entireData[i] = findActualDataset[j];

    Train(start_idx, size);
    UpdateError(start_idx, size);
}

#endif