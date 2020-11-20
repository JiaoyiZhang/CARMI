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
    // for(int i=0;i<m_dataset.size();i++)
    // {
    //     cout<<i<<":"<<m_dataset[i].first<<"\t";
    //     if((i+1)%100 == 0)
    //         cout<<endl;
    // }
    // cout<<endl;
    // use binary search to find
    // cout<<"key:"<<key<<"\tstart:"<<start_idx<<"\tend:"<<end_idx<<endl;
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        // cout<<"In while: mid:"<<mid<<"\tstart:"<<start_idx<<"\tend:"<<end_idx<<endl;
        if (m_dataset[mid].first == -1)
        {
            // cout<<"mid == -1!\tm_dataset[mid - 1].first:"<<m_dataset[mid - 1].first<<"\tkey:"<<key<<endl;
            if (m_dataset[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            // cout<<"mid != -1!\tm_dataset[mid].first:"<<m_dataset[mid].first<<"\tkey:"<<key<<endl;
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    // cout<<"Out of while! start:"<<start_idx<<"\tm_dataset[start_idx].first:"<<m_dataset[start_idx].first<<"\tm_dataset[end_idx].first:"<<m_dataset[end_idx].first<<endl;
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

inline pair<double, double> TestArrayFind(ArrayType &node, double key)
{
    if (node.m_datasetSize == 0)
        return {};
    auto entireIdx = node.datasetIndex;
    int preIdx = node.model.Predict(key);
    if (entireDataset[entireIdx][preIdx].first == key)
    {
        return entireDataset[entireIdx][preIdx];
    }
    else
    {
        int start = max(0, preIdx - node.error);
        int end = min(node.m_datasetSize - 1, preIdx + node.error);
        start = min(start, end);
        int res;
        if (key <= entireDataset[entireIdx][start].first)
            res = ArrayBinarySearch(entireDataset[entireIdx], key, 0, start);
        else if (key <= entireDataset[entireIdx][end].first)
            res = ArrayBinarySearch(entireDataset[entireIdx], key, start, end);
        else
        {
            res = ArrayBinarySearch(entireDataset[entireIdx], key, end, node.m_datasetSize - 1);
            if (res >= node.m_datasetSize)
                return {};
        }

        if (entireDataset[entireIdx][res].first == key)
            return entireDataset[entireIdx][res];
        return {};
    }
}

inline pair<double, double> TestGappedArrayFind(GappedArrayType &node, double key)
{
    auto entireIdx = node.datasetIndex;
    int preIdx = node.model.Predict(key);
    if (entireDataset[entireIdx][preIdx].first == key)
        return entireDataset[entireIdx][preIdx];
    else
    {
        int start = max(0, preIdx - node.error);
        int end = min(node.maxIndex, preIdx + node.error);
        start = min(start, end);

        int res;
        if (entireDataset[entireIdx][start].first == -1)
            start--;
        if (entireDataset[entireIdx][end].first == -1)
            end--;
        if (key <= entireDataset[entireIdx][start].first)
            res = GABinarySearch(entireDataset[entireIdx], key, 0, start);
        else if (key <= entireDataset[entireIdx][end].first)
            res = GABinarySearch(entireDataset[entireIdx], key, start, end);
        else
        {
            res = GABinarySearch(entireDataset[entireIdx], key, end, node.maxIndex - 1);
            if (res >= node.maxIndex)
                return {DBL_MIN, DBL_MIN};
        }

        if (entireDataset[entireIdx][res].first == key)
            return entireDataset[entireIdx][res];
        return {DBL_MIN, DBL_MIN};
    }
}

inline bool TestArrayInsert(ArrayType &node, pair<double, double> data)
{
    auto entireIdx = node.datasetIndex;
    if (node.m_datasetSize == 0)
    {
        entireDataset[entireIdx].push_back(data);
        node.m_datasetSize++;
        node.writeTimes++;
        node.SetDataset(entireDataset[entireIdx]);
        return true;
    }
    int preIdx = node.model.Predict(data.first);
    int start = max(0, preIdx - node.error);
    int end = min(node.m_datasetSize - 1, preIdx + node.error);
    start = min(start, end);

    if (data.first <= entireDataset[entireIdx][start].first)
        preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, 0, start);
    else if (data.first <= entireDataset[entireIdx][end].first)
        preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, start, end);
    else
        preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, end, node.m_datasetSize - 1);

    // Insert data
    if (preIdx == node.m_datasetSize - 1 && entireDataset[entireIdx][preIdx].first < data.first)
    {
        entireDataset[entireIdx].push_back(data);
        node.m_datasetSize++;
        node.writeTimes++;
        return true;
    }
    entireDataset[entireIdx].push_back(entireDataset[entireIdx][node.m_datasetSize - 1]);
    node.m_datasetSize++;
    for (int i = node.m_datasetSize - 2; i > preIdx; i--)
        entireDataset[entireIdx][i] = entireDataset[entireIdx][i - 1];
    entireDataset[entireIdx][preIdx] = data;

    node.writeTimes++;

    // If the current number is greater than the maximum,
    // the child node needs to be retrained
    if (node.writeTimes >= node.m_datasetSize || node.writeTimes > node.m_maxNumber)
        node.SetDataset(entireDataset[entireIdx]);
    return true;
}

inline bool TestGappedArrayInsert(GappedArrayType &node, pair<double, double> data)
{
    auto entireIdx = node.datasetIndex;
    if ((float(node.m_datasetSize) / node.capacity > node.density))
    {
        // If an additional Insertion results in crossing the density
        // then we expand the gapped array
        node.SetDataset(entireDataset[entireIdx]);
    }

    if (node.m_datasetSize == 0)
    {
        entireDataset[entireIdx] = vector<pair<double, double>>(node.capacity, pair<double, double>{-1, -1});
        entireDataset[entireIdx][0] = data;
        node.m_datasetSize++;
        node.maxIndex = 0;
        node.SetDataset(entireDataset[entireIdx]);
        return true;
    }
    int preIdx = node.model.Predict(data.first);

    int start = max(0, preIdx - node.error);
    int end = min(node.maxIndex, preIdx + node.error);
    start = min(start, end);

    if (entireDataset[entireIdx][start].first == -1)
        start--;
    if (entireDataset[entireIdx][end].first == -1)
        end--;

    if (data.first <= entireDataset[entireIdx][start].first)
        preIdx = GABinarySearch(entireDataset[entireIdx], data.first, 0, start);
    else if (data.first <= entireDataset[entireIdx][end].first)
        preIdx = GABinarySearch(entireDataset[entireIdx], data.first, start, end);
    else
        preIdx = GABinarySearch(entireDataset[entireIdx], data.first, end, node.maxIndex);

    // if the Insertion position is a gap,
    //  then we Insert the element into the gap and are done
    if (entireDataset[entireIdx][preIdx].first == -1)
    {
        entireDataset[entireIdx][preIdx] = data;
        node.m_datasetSize++;
        node.maxIndex = max(node.maxIndex, preIdx);
        return true;
    }
    else
    {
        if (entireDataset[entireIdx][preIdx].second == DBL_MIN)
        {
            entireDataset[entireIdx][preIdx] = data;
            node.m_datasetSize++;
            node.maxIndex = max(node.maxIndex, preIdx);
            return true;
        }
        if (preIdx == node.maxIndex && entireDataset[entireIdx][node.maxIndex].first < data.first)
        {
            entireDataset[entireIdx][++node.maxIndex] = data;
            node.m_datasetSize++;
            return true;
        }
        // If the Insertion position is not a gap, we make
        // a gap at the Insertion position by shifting the elements
        // by one position in the direction of the closest gap

        int i = preIdx + 1;
        while (entireDataset[entireIdx][i].first != -1)
            i++;
        if (i >= node.capacity)
        {
            i = preIdx - 1;
            while (i >= 0 && entireDataset[entireIdx][i].first != -1)
                i--;
            for (int j = i; j < preIdx - 1; j++)
                entireDataset[entireIdx][j] = entireDataset[entireIdx][j + 1];
            preIdx--;
        }
        else
        {
            if (i > node.maxIndex)
                node.maxIndex++;
            for (; i > preIdx; i--)
                entireDataset[entireIdx][i] = entireDataset[entireIdx][i - 1];
        }
        entireDataset[entireIdx][preIdx] = data;
        node.m_datasetSize++;
        node.maxIndex = max(node.maxIndex, preIdx);
        return true;
    }
    return false;
}

#endif