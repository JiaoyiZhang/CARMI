#ifndef GA_TYPE_H
#define GA_TYPE_H

#include <float.h>
#include <vector>
#include "../params.h"
#include "../dataManager/datapoint.h"
#include "../baseNode.h"
#include "ga.h"
using namespace std;

extern pair<double, double> *entireData;
extern vector<pair<double, double>> findDatapoint;

inline void GappedArrayType::SetDataset(const vector<pair<double, double>> &subDataset, int cap)
{
    if (m_left != -1)
        releaseMemory(m_left, capacity);
    capacity = cap;
    while ((float(subDataset.size()) / float(capacity) >= density))
        capacity = float(capacity) / density + 1;
    capacity *= 2; // test
    if (capacity > 4096)
        capacity = 4096;
    int size = 0;
    m_left = allocateMemory(capacity);

    int k = density / (1 - density);
    int cnt = 0;
    vector<pair<double, double>> newDataset(capacity, pair<double, double>{-1, -1});
    int j = 0;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if ((subDataset[i].first != -1) && (subDataset[i].second != DBL_MIN))
        {
            cnt++;
            if (cnt > k)
            {
                j++;
                cnt = 0;
            }
            newDataset[j++] = subDataset[i];
            maxIndex = j - 1;
            size++;
        }
    }

    if (size > 4096)
        cout << "Gapped Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    for (int i = m_left, j = 0; j < capacity; i++, j++)
        entireData[i] = newDataset[j];
    flagNumber += size;

    Train(newDataset);

    UpdateError(newDataset);
}

inline void GappedArrayType::SetDataset(const int left, const int size, int cap)
{
    vector<pair<double, double>> newDataset;
    int right = left + maxIndex + 1;
    for (int i = left; i < right; i++)
        newDataset.push_back(entireData[i]);
    SetDataset(newDataset, cap);
}

inline int GappedArrayType::UpdateError(const vector<pair<double, double>> &newDataset)
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    for (int i = 0; i < newDataset.size(); i++)
    {
        if (newDataset[i].first != -1)
        {
            p = Predict(newDataset[i].first);
            d = abs(i - p);
            if (d > maxError)
                maxError = d;
        }
    }

    // find the optimal value of error
    int minRes = newDataset.size() * log(newDataset.size()) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = 0; i < newDataset.size(); i++)
        {
            if (newDataset[i].first != -1)
            {
                p = Predict(newDataset[i].first);
                d = abs(i - p);
                if (d <= e)
                    cntBetween++;
                else
                    cntOut++;
            }
        }
        if (e != 0)
            res = cntBetween * log(e) / log(2) + cntOut * log(newDataset.size()) / log(2);
        else
            res = cntOut * log(newDataset.size()) / log(2);
        if (res < minRes)
        {
            minRes = res;
            error = e;
        }
    }
    return error;
}

inline void GappedArrayType::Train(const vector<pair<double, double>> &dataset)
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

inline int GappedArrayType::Predict(double key)
{
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0xFFFFFF);
    int p = (theta1 * key + theta2) * maxIndex;
    if (p < 0)
        p = 0;
    else if (p >= maxIndex)
        p = maxIndex;
    return p;
}

inline int GappedArrayType::UpdateError(const int start_idx, const int size)
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
    error *= (2 - density);
    return error;
}

inline void GappedArrayType::Train(const int start_idx, const int size)
{
    vector<double> index;
    for (int i = start_idx; i < start_idx + size; i++)
        index.push_back(double(i - start_idx) / double(size));

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = start_idx; i < start_idx + size; i++)
    {
        t1 += findDatapoint[i].first * findDatapoint[i].first;
        t2 += findDatapoint[i].first;
        t3 += findDatapoint[i].first * index[i - start_idx];
        t4 += index[i - start_idx];
    }
    theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
    maxIndex = size;
}

#endif