#ifndef GA_TYPE_H
#define GA_TYPE_H

#include <float.h>
#include <vector>
#include "../../carmi.h"
#include "ga.h"
using namespace std;

inline int GappedArrayType::Predict(double key) const
{
    // return the predicted idx in the leaf node
    int size = (flagNumber & 0x00FFFFFF);
    int p = (theta1 * key + theta2) * maxIndex;
    if (p < 0)
        p = 0;
    else if (p >= maxIndex)
        p = maxIndex;
    return p;
}

inline void CARMI::initGA(GappedArrayType *ga, const vector<pair<double, double>> &subDataset, int cap)
{
    if (ga->m_left != -1)
        releaseMemory(ga->m_left, ga->capacity);
    ga->capacity = cap;
    while ((float(subDataset.size()) / float(ga->capacity) > ga->density))
        ga->capacity = float(ga->capacity) / ga->density + 1;
    // ga->capacity *= 2; // test
    if (ga->capacity > 4096)
        ga->capacity = 4096;
    int size = 0;
    ga->m_left = allocateMemory(ga->capacity);

    int k = ga->density / (1 - ga->density);
    float rate = float(subDataset.size()) / ga->capacity;
    if (rate > ga->density)
        k = rate / (1 - rate);
    int cnt = 0;
    vector<pair<double, double>> newDataset(ga->capacity, pair<double, double>{DBL_MIN, DBL_MIN});
    int j = 0;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if (subDataset[i].second != DBL_MIN)
        {
            if (cnt >= k)
            {
                j++;
                cnt = 0;
                if (j > 2048)
                    k += 2;
            }
            cnt++;
            newDataset[j++] = subDataset[i];
            ga->maxIndex = j - 1;
            size++;
        }
    }

    if (size > 4096)
        cout << "Gapped Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    for (int i = ga->m_left, j = 0; j <= ga->maxIndex; i++, j++)
        entireData[i] = newDataset[j];
    ga->flagNumber += size;

    Train(ga);

    UpdateError(ga);
}

inline void CARMI::initGA(GappedArrayType *ga, const int left, const int size, int cap)
{
    vector<pair<double, double>> newDataset;
    int right = left + ga->maxIndex + 1;
    for (int i = left; i < right; i++)
        newDataset.push_back(entireData[i]);
    initGA(ga, newDataset, cap);
}

inline void CARMI::initGA(GappedArrayType *ga, const int left, const int size)
{
    if (ga->m_left != -1)
        releaseMemory(ga->m_left, ga->capacity);
    while ((float(size) / float(ga->capacity) > ga->density))
        ga->capacity = float(ga->capacity) / ga->density + 1;
    if (ga->capacity > 4096)
        ga->capacity = 4096;
    ga->m_left = allocateMemory(ga->capacity);

    if (size > 4096)
        cout << "Gapped Array setDataset WRONG! datasetSize > 4096, size is:" << size << endl;

    int k = ga->density / (1 - ga->density);
    float rate = float(size) / ga->capacity;
    if (rate > ga->density)
        k = rate / (1 - rate);
    int cnt = 0;
    int j = ga->m_left;
    int end = left + size;
    for (int i = left; i < end; i++)
    {
        if (cnt >= k)
        {
            j++;
            cnt = 0;
        }
        cnt++;
        entireData[j++] = initDataset[i];
        ga->maxIndex = j - 1 - ga->m_left;
    }

    ga->flagNumber += size;

    Train(ga);
    UpdateError(ga);
}

inline int CARMI::UpdateError(GappedArrayType *ga)
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    int end = ga->m_left + ga->capacity;
    for (int i = ga->m_left; i < end; i++)
    {
        if (entireData[i].first != DBL_MIN)
        {
            p = ga->Predict(entireData[i].first) + ga->m_left;
            d = abs(i - p);
            if (d > maxError)
                maxError = d;
        }
    }

    // find the optimal value of ga->error
    int minRes = ga->maxIndex * log(ga->maxIndex) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = ga->m_left; i < end; i++)
        {
            if (entireData[i].first != DBL_MIN)
            {
                p = ga->Predict(entireData[i].first) + ga->m_left;
                d = abs(i - p);
                if (d <= e)
                    cntBetween++;
                else
                    cntOut++;
            }
        }
        if (e != 0)
            res = cntBetween * log(e) / log(2) + cntOut * log(ga->maxIndex) / log(2);
        else
            res = cntOut * log(ga->maxIndex) / log(2);
        if (res < minRes)
        {
            minRes = res;
            ga->error = e;
        }
    }
    return ga->error;
}

inline int CARMI::UpdateError(GappedArrayType *ga, const int start_idx, const int size)
{
    // find: max|pi-yi|
    int maxError = 0, p, d;
    for (int i = start_idx; i < start_idx + size; i++)
    {
        p = ga->Predict(initDataset[i].first);
        d = abs(i - start_idx - p);
        if (d > maxError)
            maxError = d;
    }

    // find the optimal value of ga->error
    int minRes = size * log(size) / log(2);
    int res;
    int cntBetween, cntOut;
    for (int e = 0; e <= maxError; e++)
    {
        cntBetween = 0;
        cntOut = 0;
        for (int i = start_idx; i < start_idx + size; i++)
        {
            p = ga->Predict(initDataset[i].first);
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
            ga->error = e;
        }
    }
    ga->error *= (2 - ga->density);
    return ga->error;
}

inline void CARMI::Train(GappedArrayType *ga)
{
    int actualSize = 0;
    vector<double> index;
    int end = ga->m_left + ga->capacity;
    int size = ga->flagNumber & 0x00FFFFFF;
    for (int i = ga->m_left; i < end; i++)
    {
        if (entireData[i].first != DBL_MIN)
            actualSize++;
        index.push_back(double(i - ga->m_left) / double(ga->maxIndex));
    }
    if (actualSize == 0)
        return;

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = ga->m_left; i < end; i++)
    {
        if (entireData[i].first != DBL_MIN)
        {
            t1 += entireData[i].first * entireData[i].first;
            t2 += entireData[i].first;
            t3 += entireData[i].first * index[i - ga->m_left];
            t4 += index[i - ga->m_left];
        }
    }
    ga->theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
    ga->theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
}

inline void CARMI::Train(GappedArrayType *ga, const int start_idx, const int size)
{
    if ((ga->flagNumber & 0x00FFFFFF) != size)
        ga->flagNumber += size;
    vector<double> index;
    for (int i = start_idx; i < start_idx + size; i++)
        index.push_back(double(i - start_idx) / double(size));

    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = start_idx; i < start_idx + size; i++)
    {
        t1 += initDataset[i].first * initDataset[i].first;
        t2 += initDataset[i].first;
        t3 += initDataset[i].first * index[i - start_idx];
        t4 += index[i - start_idx];
    }
    ga->theta1 = (t3 * size - t2 * t4) / (t1 * size - t2 * t2);
    ga->theta2 = (t1 * t4 - t2 * t3) / (t1 * size - t2 * t2);
    ga->maxIndex = size;
}

#endif