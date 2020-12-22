#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H

#include "../params.h"
#include "../trainModel/lr.h"
#include <vector>
#include "../dataManager/datapoint.h"
using namespace std;

extern pair<double, double> *entireData;
class ArrayType
{
public:
    ArrayType(){};
    ArrayType(int cap)
    {
        m_datasetSize = 0;
        error = 0;
        m_left = -1;
        m_capacity = cap; // 256 or 512
    }
    inline int UpdateError(const vector<pair<double, double>> &dataset);
    inline void SetDataset(const vector<pair<double, double>> &dataset, int cap);
    inline void SetDataset(const int left, const int size, int cap);

    LinearRegression model; // 20 Byte
    int m_left;             // the left boundary of the leaf node in the global array
    int m_datasetSize;      // the current amount of data
    int m_capacity;         // the maximum capacity of this leaf node

    int error; // the boundary of binary search
};

inline void ArrayType::SetDataset(const vector<pair<double, double>> &dataset, int cap)
{
    if (m_left != -1)
        releaseMemory(m_left, m_capacity);
    m_capacity = cap;
    m_datasetSize = dataset.size();
    while (m_datasetSize >= m_capacity)
        m_capacity *= kExpansionScale;

    m_capacity *= 2; // test
    if (m_capacity > 4096)
        m_capacity = 4096;

    m_left = allocateMemory(m_capacity);
    if (m_datasetSize == 0)
        return;

    if (m_datasetSize > 4096)
        cout << "Array setDataset WRONG! datasetSize > 4096, size is:" << m_datasetSize << endl;

    for (int i = m_left, j = 0; j < m_datasetSize; i++, j++)
        entireData[i] = dataset[j];

    model.Train(dataset, m_datasetSize);
    UpdateError(dataset);
}

inline void ArrayType::SetDataset(const int left, const int size, int cap)
{
    m_datasetSize = size;
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
    for (int i = 0; i < m_datasetSize; i++)
    {
        p = model.PredictPrecision(dataset[i].first, m_datasetSize);
        d = abs(i - p);
        if (d > maxError)
            maxError = d;
    }

    // find the optimal value of error
    int minRes = m_datasetSize * log(m_datasetSize) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = 0; i < m_datasetSize; i++)
        {
            p = model.PredictPrecision(dataset[i].first, m_datasetSize);
            d = abs(i - p);
            if (d <= e)
                cntBetween++;
            else
                cntOut++;
        }
        if (e != 0)
            res = cntBetween * log(e) / log(2) + cntOut * log(m_datasetSize) / log(2);
        else
            res = cntOut * log(m_datasetSize) / log(2);
        if (res < minRes)
        {
            minRes = res;
            error = e;
        }
    }
    return error;
}

#endif