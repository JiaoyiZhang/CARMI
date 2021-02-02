#ifndef YCSB_LEAF_TYPE_H
#define YCSB_LEAF_TYPE_H

#include <vector>
#include "../params.h"
#include "../dataManager/datapoint.h"
#include "../baseNode.h"
#include "ycsb_leaf.h"
using namespace std;

extern vector<pair<double, double>> findActualDataset;

inline int YCSBLeaf::Predict(double key)
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

inline int YCSBLeaf::UpdateError(const int start_idx, const int size)
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

inline void YCSBLeaf::Train(const int start_idx, const int size)
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

inline void YCSBLeaf::SetDataset(const int start_idx, const int size)
{
    flagNumber += size;
    m_left = start_idx;
    if (size == 0)
        return;

    Train(start_idx, size);
    UpdateError(start_idx, size);
}

#endif // !YCSB_LEAF_TYPE_H