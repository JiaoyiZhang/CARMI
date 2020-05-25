#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include "leaf_node.h"
#include <float.h>
#include <algorithm>

class GappedArray : public BasicLeafNode
{
public:
    GappedArray(int cap) : BasicLeafNode()
    {
        density = 0.5;
        capacity = cap;
        maxIndex = -2;
    }
    void SetDataset(const vector<pair<double, double>> &subDataset);

    pair<double, double> Find(double key);
    bool Insert(pair<double, double> data);
    bool Update(pair<double, double> data);
    bool Delete(double key);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset);

    int BinarySearch(double key, int p, int start, int end);
    int ExponentialSearch(double key, int p, int start, int end);

    // designed for gtest
    int GetPredictIndex(double key)
    {
        double p = model->Predict(key);
        int preIdx = static_cast<int>(p * (capacity - 1));
        return preIdx;
    }
    int GetMaxIndex(){return maxIndex;}

private:
    int maxIndex;   // tht index of the last one
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
};

void GappedArray::SetDataset(const vector<pair<double, double>> &subDataset)
{
    while ((float(subDataset.size()) / float(capacity) > density))
    {
        int newSize = capacity / density;
        capacity = newSize;
    }
    m_datasetSize = 0;

    vector<pair<double, double>> newDataset(capacity, pair<double, double>{-1, -1});
    maxIndex = -2;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if ((subDataset[i].first != -1) && (subDataset[i].second != DBL_MIN))
        {
            maxIndex += 2;
            newDataset[maxIndex] = subDataset[i];
            m_datasetSize++;
        }
    }
    m_dataset = newDataset;
    model->Train(m_dataset);
    for (int i = 0; i < m_dataset.size(); i++)
    {
        if (m_dataset[i].first != -1)
        {
            double p = model->Predict(m_dataset[i].first);
            int preIdx = static_cast<int>(p * (capacity - 1));
            int error = i - preIdx;
            if (error > maxPositiveError)
                maxPositiveError = error;
            if (error < maxNegativeError)
                maxNegativeError = error;
        }
    }
    maxPositiveError++;
    maxNegativeError--;
}

pair<double, double> GappedArray::Find(double key)
{
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (capacity - 1));
    if (m_dataset[preIdx].first == key)
        return m_dataset[preIdx];
    else
    {
        int start = max(0, preIdx + maxNegativeError);
        int end = min(maxIndex, preIdx + maxPositiveError);
        start = min(start, end);
        int res = SEARCH_METHOD(key, preIdx, start, end);

        if (res <= start)
            res = SEARCH_METHOD(key, preIdx, 0, start);
        else if (res >= end)
        {
            res = SEARCH_METHOD(key, preIdx, res, maxIndex);
            if (res > maxIndex)
                return {DBL_MIN, DBL_MIN};
        }
        if (m_dataset[res].first == key)
            return m_dataset[res];
        return {DBL_MIN, DBL_MIN};
    }
}

bool GappedArray::Update(pair<double, double> data)
{
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (capacity - 1));
    if (m_dataset[preIdx].first == data.first)
    {
        m_dataset[preIdx].second = data.second;
        return true;
    }
    else
    {
        int start = max(0, preIdx + maxNegativeError);
        int end = min(maxIndex, preIdx + maxPositiveError);
        start = min(start, end);
        int res = SEARCH_METHOD(data.first, preIdx, start, end);
        if (res <= start)
            res = SEARCH_METHOD(data.first, preIdx, 0, start);
        else if (res >= end)
        {
            res = SEARCH_METHOD(data.first, preIdx, res, maxIndex);
            if (res > maxIndex)
                return false;
        }
        if (m_dataset[res].first != data.first)
            return false;
        m_dataset[res].second = data.second;
        return true;
    }
}

bool GappedArray::Delete(double key)
{
    // DBL_MIN means the data has been deleted
    // when a data has been deleted, data.second == DBL_MIN
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (capacity - 1));
    if (m_dataset[preIdx].first == key)
    {
        m_dataset[preIdx].second = DBL_MIN;
        m_datasetSize--;
        if (preIdx == maxIndex)
            maxIndex--;
        return true;
    }
    else
    {
        int start = max(0, preIdx + maxNegativeError);
        int end = min(maxIndex, preIdx + maxPositiveError);
        start = min(start, end);
        int res = SEARCH_METHOD(key, preIdx, start, end);

        if (res <= start)
            res = SEARCH_METHOD(key, preIdx, 0, start);
        else if (res >= end)
        {
            res = SEARCH_METHOD(key, preIdx, res, maxIndex);
            if (res > maxIndex)
                return false;
        }
        if (m_dataset[res].first != key)
            return false;
        m_datasetSize--;
        m_dataset[res].second = DBL_MIN;
        if (res == maxIndex)
            maxIndex--;
        return true;
    }
}

bool GappedArray::Insert(pair<double, double> data)
{
    if ((float(m_datasetSize) / capacity > density))
    {
        // If an additional Insertion results in crossing the density
        // then we expand the gapped array
        SetDataset(m_dataset);
    }

    if (m_datasetSize == 0)
    {
        m_dataset = vector<pair<double, double>>(capacity, pair<double, double>{-1, -1});
        m_dataset[0] = data;
        m_datasetSize++;
        maxIndex = 0;
        SetDataset(m_dataset);
        return true;
    }
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (maxIndex + 2));

    int start = max(0, preIdx + maxNegativeError);
    int end = min(maxIndex, preIdx + maxPositiveError);
    start = min(start, end);
    preIdx = SEARCH_METHOD(data.first, preIdx, start, end);
    if (preIdx <= start)
        preIdx = SEARCH_METHOD(data.first, preIdx, 0, start);
    else if (preIdx >= end)
        preIdx = SEARCH_METHOD(data.first, preIdx, preIdx, maxIndex);

    // if the Insertion position is a gap,
    //  then we Insert the element into the gap and are done
    if (m_dataset[preIdx].first == -1)
    {
        m_dataset[preIdx] = data;
        m_datasetSize++;
        maxIndex = max(maxIndex, preIdx);
        return true;
    }
    else
    {
        if (m_dataset[preIdx].second == DBL_MIN)
        {
            m_dataset[preIdx] = data;
            m_datasetSize++;
            maxIndex = max(maxIndex, preIdx);
            return true;
        }
        if (preIdx == maxIndex && m_dataset[maxIndex].first < data.first)
        {
            m_dataset[++maxIndex] = data;
            m_datasetSize++;
            return true;
        }
        // If the Insertion position is not a gap, we make
        // a gap at the Insertion position by shifting the elements
        // by one position in the direction of the closest gap

        int i = preIdx + 1;
        while (m_dataset[i].first != -1)
            i++;
        if (i >= capacity)
        {
            i = preIdx - 1;
            while (i >= 0 && m_dataset[i].first != -1)
                i--;
            for (int j = i; j < preIdx - 1; j++)
                m_dataset[j] = m_dataset[j + 1];
            preIdx--;
        }
        else
        {
            if (i > maxIndex)
                maxIndex++;
            for (i; i > preIdx; i--)
                m_dataset[i] = m_dataset[i - 1];
        }
        m_dataset[preIdx] = data;
        m_datasetSize++;
        maxIndex = max(maxIndex, preIdx);
        return true;
    }
    return false;
}

int GappedArray::BinarySearch(double key, int preIdx, int start_idx, int end_idx)
{
    // use binary search to find
    while (start_idx < end_idx)
    {
        if (m_dataset[start_idx].first == -1)
            start_idx--;
        if (m_dataset[end_idx].first == -1)
            end_idx++;
        int mid = (start_idx + end_idx) >> 1;
        if (m_dataset[mid].first == -1)
        {
            int left = max(start_idx, mid - 1);
            int right = min(end_idx, mid + 1);
            if (m_dataset[left].first >= key)
                end_idx = left;
            else
                start_idx = right;
        }
        else
        {
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    return start_idx;
}
/*
search the index of key
0. Use p to get a approximate position of key
1. Exponential Search at first, and decide the range of binary search
2. Use Binary Search to search the gap array.
*/
int GappedArray::ExponentialSearch(double key, int p, int start, int end)
{
    // exponential search
    int offset = 1;
    int i = start;
    if (m_dataset[i].first == -1)
    {
        i--;
        if (i < 0)
            i = 1;
    }
    while (i <= end && m_dataset[i].first < key)
    {
        i = start + offset;
        offset = offset << 1;
        if (m_dataset[i].first == -1)
            i++;
    }
    start = (start + offset >> 2) > maxIndex ? maxIndex : (start + offset >> 2);
    end = min(i, maxIndex);
    if (m_dataset[start].first == -1)
        start++;
    return BinarySearch(key, p, start, end);
}

long double GappedArray::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset)
{
    int datasetSize = dataset.size();
    if (datasetSize == 0)
        return 0;

    // calculate cost
    long double totalCost = 0;
    double READCOST = log(datasetSize) / log(6);
    double WRITECOST = 3.5 * READCOST;
    for (int i = 0; i < datasetSize; i++)
    {
        pair<int, int> tmp = (cntTree.find(dataset[i].first))->second;
        double tmpRead = tmp.first * READCOST;
        double tmpWrite = tmp.first * WRITECOST;
        totalCost += tmpRead + tmpWrite;
    }
    return totalCost;
}


#endif