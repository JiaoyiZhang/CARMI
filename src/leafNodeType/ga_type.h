#ifndef GA_TYPE_H
#define GA_TYPE_H

#include "../params.h"
#include "../trainModel/lr.h"
#include <float.h>
#include <vector>
#include "../datapoint.h"
using namespace std;

extern pair<double, double> *entireData;
class GappedArrayType
{
public:
    GappedArrayType(){};
    GappedArrayType(int cap)
    {
        m_datasetSize = 0;
        error = 0;
        density = kDensity;
        capacity = cap;
        maxIndex = -2;
        m_left = -1;
    }
    inline int UpdateError(const vector<pair<double, double>> &dataset);
    inline void SetDataset(const vector<pair<double, double>> &dataset, int cap);
    inline void SetDataset(const int left, const int size, int cap);

    LinearRegression model;
    int m_left;        // the left boundary of the leaf node in the global array
    int m_datasetSize; // the current amount of data
    int capacity;      // the current maximum capacity of the leaf node

    int maxIndex;   // tht index of the last one
    int error;      // the boundary of binary search
    double density; // the maximum density of the leaf node data
};

inline void GappedArrayType::SetDataset(const vector<pair<double, double>> &subDataset, int cap)
{
    if (m_left != -1)
        releaseMemory(m_left, capacity);
    capacity = cap;
    while ((float(subDataset.size()) / float(capacity) >= density))
        capacity = capacity / density;
    if (capacity > 4096)
        capacity = 4096;
    m_datasetSize = 0;
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
            m_datasetSize++;
        }
    }

    for (int i = m_left, j = 0; j < capacity; i++, j++)
        entireData[i] = newDataset[j];

    model.Train(newDataset, capacity);

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
            p = model.PredictPrecision(newDataset[i].first, newDataset.size());
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
        for (int i = 0; i < m_datasetSize; i++)
        {
            if (newDataset[i].first != -1)
            {
                p = model.PredictPrecision(newDataset[i].first, newDataset.size());
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
#endif