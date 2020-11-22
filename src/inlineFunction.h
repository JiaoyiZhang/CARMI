#ifndef INLINE_FUNCTION_H
#define INLINE_FUNCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include <vector>
using namespace std;
extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;

extern vector<vector<pair<double, double>>> tmpEntireDataset;

// search a key-value through binary search in
// the array leaf node
inline int ArrayBinarySearch(vector<pair<double, double>> &m_dataset, double key, int start, int end)
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (m_dataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

// search a key-value through binary search
// in the gapped array
// return the idx of the first element >= key
inline int GABinarySearch(vector<pair<double, double>> &m_dataset, double key, int start_idx, int end_idx)
{
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        if (m_dataset[mid].first == -1)
        {
            if (m_dataset[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    if (m_dataset[start_idx].first >= key)
        return start_idx;
    else
        return end_idx;
}

// designed for construction

inline void InnerNodeTime(int idx, int type, double key)
{
    int content;
    switch (type)
    {
    case 0:
    {
        content = LRVector[idx].child[LRVector[idx].model.Predict(key)];
        type = content >> 28;
        idx = content & 0x0FFFFFFF;
        return;
    }
    break;
    case 1:
    {
        content = NNVector[idx].child[NNVector[idx].model.Predict(key)];
        type = content >> 28;
        idx = content & 0x0FFFFFFF;
        return;
    }
    break;
    case 2:
    {
        content = HisVector[idx].child[HisVector[idx].model.Predict(key)];
        type = content >> 28;
        idx = content & 0x0FFFFFFF;
        return;
    }
    break;
    case 3:
    {
        content = BSVector[idx].child[BSVector[idx].model.Predict(key)];
        type = content >> 28;
        idx = content & 0x0FFFFFFF;
        return;
    }
    break;
    }
}

inline void TestArraySetDataset(ArrayType &node, const vector<pair<double, double> > &subDataset)
{
    tmpEntireDataset.push_back(subDataset);
    node.datasetIndex = tmpEntireDataset.size() - 1;
    node.m_datasetSize = subDataset.size();
    if (node.m_datasetSize == 0)
        return;

    node.model.Train(subDataset, node.m_datasetSize);
    int sum = 0;
    for (int i = 0; i < node.m_datasetSize; i++)
    {
        int p = node.model.Predict(subDataset[i].first);
        int e = abs(i - p);
        sum += e;
    }
    node.error = float(sum) / node.m_datasetSize + 1;
    node.writeTimes = 0;
}

inline void TestGappedArraySetDataset(GappedArrayType &node, const vector<pair<double, double> > &subDataset)
{
    while ((float(subDataset.size()) / float(node.capacity) > node.density))
    {
        int newSize = node.capacity / node.density;
        node.capacity = newSize;
    }
    node.m_datasetSize = 0;

    vector<pair<double, double> > newDataset(node.capacity, pair<double, double>{-1, -1});
    node.maxIndex = -2;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if ((subDataset[i].first != -1) && (subDataset[i].second != DBL_MIN))
        {
            node.maxIndex += 2;
            newDataset[node.maxIndex] = subDataset[i];
            node.m_datasetSize++;
        }
    }
    tmpEntireDataset.push_back(newDataset);
    node.datasetIndex = tmpEntireDataset.size() - 1;
    node.model.Train(newDataset, node.capacity);
    for (int i = 0; i < newDataset.size(); i++)
    {
        if (newDataset[i].first != -1)
        {
            int p = node.model.Predict(newDataset[i].first);
            int e = abs(i - p);
            if (e > node.error)
                node.error = e;
        }
    }
    node.error++;
}

inline pair<double, double> TestArrayFind(ArrayType &node, double key)
{
    if (node.m_datasetSize == 0)
        return {};
    auto entireIdx = node.datasetIndex;
    int preIdx = node.model.PredictPrecision(key, node.m_datasetSize);
    if (tmpEntireDataset[entireIdx][preIdx].first == key)
    {
        return tmpEntireDataset[entireIdx][preIdx];
    }
    else
    {
        int start = max(0, preIdx - node.error);
        int end = min(node.m_datasetSize - 1, preIdx + node.error);
        start = min(start, end);
        int res;
        if (key <= tmpEntireDataset[entireIdx][start].first)
            res = ArrayBinarySearch(tmpEntireDataset[entireIdx], key, 0, start);
        else if (key <= tmpEntireDataset[entireIdx][end].first)
            res = ArrayBinarySearch(tmpEntireDataset[entireIdx], key, start, end);
        else
        {
            res = ArrayBinarySearch(tmpEntireDataset[entireIdx], key, end, node.m_datasetSize - 1);
            if (res >= node.m_datasetSize)
                return {};
        }

        if (tmpEntireDataset[entireIdx][res].first == key)
            return tmpEntireDataset[entireIdx][res];
        return {};
    }
}

inline pair<double, double> TestGappedArrayFind(GappedArrayType &node, double key)
{
    auto entireIdx = node.datasetIndex;
    int preIdx = node.model.PredictPrecision(key, node.m_datasetSize);
    if (tmpEntireDataset[entireIdx][preIdx].first == key)
        return tmpEntireDataset[entireIdx][preIdx];
    else
    {
        int start = max(0, preIdx - node.error);
        int end = min(node.maxIndex, preIdx + node.error);
        start = min(start, end);

        int res;
        if (tmpEntireDataset[entireIdx][start].first == -1)
            start--;
        if (tmpEntireDataset[entireIdx][end].first == -1)
            end--;
        if (key <= tmpEntireDataset[entireIdx][start].first)
            res = GABinarySearch(tmpEntireDataset[entireIdx], key, 0, start);
        else if (key <= tmpEntireDataset[entireIdx][end].first)
            res = GABinarySearch(tmpEntireDataset[entireIdx], key, start, end);
        else
        {
            res = GABinarySearch(tmpEntireDataset[entireIdx], key, end, node.maxIndex - 1);
            if (res >= node.maxIndex)
                return {DBL_MIN, DBL_MIN};
        }

        if (tmpEntireDataset[entireIdx][res].first == key)
            return tmpEntireDataset[entireIdx][res];
        return {DBL_MIN, DBL_MIN};
    }
}

inline bool TestArrayInsert(ArrayType &node, pair<double, double> data)
{
    auto entireIdx = node.datasetIndex;
    if (node.m_datasetSize == 0)
    {
        tmpEntireDataset[entireIdx].push_back(data);
        node.m_datasetSize++;
        node.writeTimes++;
        node.SetDataset(tmpEntireDataset[entireIdx]);
        return true;
    }
    int preIdx = node.model.PredictPrecision(data.first, node.m_datasetSize);
    int start = max(0, preIdx - node.error);
    int end = min(node.m_datasetSize - 1, preIdx + node.error);
    start = min(start, end);

    if (data.first <= tmpEntireDataset[entireIdx][start].first)
        preIdx = ArrayBinarySearch(tmpEntireDataset[entireIdx], data.first, 0, start);
    else if (data.first <= tmpEntireDataset[entireIdx][end].first)
        preIdx = ArrayBinarySearch(tmpEntireDataset[entireIdx], data.first, start, end);
    else
        preIdx = ArrayBinarySearch(tmpEntireDataset[entireIdx], data.first, end, node.m_datasetSize - 1);

    // Insert data
    if (preIdx == node.m_datasetSize - 1 && tmpEntireDataset[entireIdx][preIdx].first < data.first)
    {
        tmpEntireDataset[entireIdx].push_back(data);
        node.m_datasetSize++;
        node.writeTimes++;
        return true;
    }
    tmpEntireDataset[entireIdx].push_back(tmpEntireDataset[entireIdx][node.m_datasetSize - 1]);
    node.m_datasetSize++;
    for (int i = node.m_datasetSize - 2; i > preIdx; i--)
        tmpEntireDataset[entireIdx][i] = tmpEntireDataset[entireIdx][i - 1];
    tmpEntireDataset[entireIdx][preIdx] = data;

    node.writeTimes++;

    // If the current number is greater than the maximum,
    // the child node needs to be retrained
    if (node.writeTimes >= node.m_datasetSize || node.writeTimes > node.m_maxNumber)
        node.SetDataset(tmpEntireDataset[entireIdx]);
    return true;
}

inline bool TestGappedArrayInsert(GappedArrayType &node, pair<double, double> data)
{
    auto entireIdx = node.datasetIndex;
    if ((float(node.m_datasetSize) / node.capacity > node.density))
    {
        // If an additional Insertion results in crossing the density
        // then we expand the gapped array
        node.SetDataset(tmpEntireDataset[entireIdx]);
    }

    if (node.m_datasetSize == 0)
    {
        tmpEntireDataset[entireIdx] = vector<pair<double, double>>(node.capacity, pair<double, double>{-1, -1});
        tmpEntireDataset[entireIdx][0] = data;
        node.m_datasetSize++;
        node.maxIndex = 0;
        node.SetDataset(tmpEntireDataset[entireIdx]);
        return true;
    }
    int preIdx = node.model.PredictPrecision(data.first, node.m_datasetSize);

    int start = max(0, preIdx - node.error);
    int end = min(node.maxIndex, preIdx + node.error);
    start = min(start, end);

    if (tmpEntireDataset[entireIdx][start].first == -1)
        start--;
    if (tmpEntireDataset[entireIdx][end].first == -1)
        end--;

    if (data.first <= tmpEntireDataset[entireIdx][start].first)
        preIdx = GABinarySearch(tmpEntireDataset[entireIdx], data.first, 0, start);
    else if (data.first <= tmpEntireDataset[entireIdx][end].first)
        preIdx = GABinarySearch(tmpEntireDataset[entireIdx], data.first, start, end);
    else
        preIdx = GABinarySearch(tmpEntireDataset[entireIdx], data.first, end, node.maxIndex);

    // if the Insertion position is a gap,
    //  then we Insert the element into the gap and are done
    if (tmpEntireDataset[entireIdx][preIdx].first == -1)
    {
        tmpEntireDataset[entireIdx][preIdx] = data;
        node.m_datasetSize++;
        node.maxIndex = max(node.maxIndex, preIdx);
        return true;
    }
    else
    {
        if (tmpEntireDataset[entireIdx][preIdx].second == DBL_MIN)
        {
            tmpEntireDataset[entireIdx][preIdx] = data;
            node.m_datasetSize++;
            node.maxIndex = max(node.maxIndex, preIdx);
            return true;
        }
        if (preIdx == node.maxIndex && tmpEntireDataset[entireIdx][node.maxIndex].first < data.first)
        {
            tmpEntireDataset[entireIdx][++node.maxIndex] = data;
            node.m_datasetSize++;
            return true;
        }
        // If the Insertion position is not a gap, we make
        // a gap at the Insertion position by shifting the elements
        // by one position in the direction of the closest gap

        int i = preIdx + 1;
        while (tmpEntireDataset[entireIdx][i].first != -1)
            i++;
        if (i >= node.capacity)
        {
            i = preIdx - 1;
            while (i >= 0 && tmpEntireDataset[entireIdx][i].first != -1)
                i--;
            for (int j = i; j < preIdx - 1; j++)
                tmpEntireDataset[entireIdx][j] = tmpEntireDataset[entireIdx][j + 1];
            preIdx--;
        }
        else
        {
            if (i > node.maxIndex)
                node.maxIndex++;
            for (; i > preIdx; i--)
                tmpEntireDataset[entireIdx][i] = tmpEntireDataset[entireIdx][i - 1];
        }
        tmpEntireDataset[entireIdx][preIdx] = data;
        node.m_datasetSize++;
        node.maxIndex = max(node.maxIndex, preIdx);
        return true;
    }
    return false;
}

#endif