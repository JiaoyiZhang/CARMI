#ifndef GAPPED_NODE_H
#define GAPPED_NODE_H

#include "../params.h"
#include "../../cpp-btree/btree_map.h"
// #include "../trainModel/nn.h"
// #include "../trainModel/lr.h"
#include <array>

static const int childNumber = 20;

template <typename type>
class gappedNode
{
public:
    gappedNode(int threshold, params secondStageParams, int cap)
    {
        maxKeyNum = threshold;
        m_secondStageParams = secondStageParams;
        m_datasetSize = 0;
        density = 0.75;
        capacity = cap;
        isLeafNode = true;
    }

    void train(const vector<pair<double, double>> &subDataset);
    int getSize() { return m_datasetSize; }
    bool isLeaf() { return isLeafNode; }
    vector<pair<double, double>> &getDataset() { return m_dataset; }

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool update(pair<double, double> data);
    bool del(double key);

private:
    int search(double key, int p);                                                                    // return the index of key
    void insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt); // insert data into new vector

    void expand(); // expand the vector when m_datasetSize / capacity >= density

private:
    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    params m_secondStageParams; // parameters of network
    type m_secondStageNetwork = type();

    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data

    bool isLeafNode;
};

template <typename type>
void gappedNode<type>::train(const vector<pair<double, double>> &subDataset)
{
    m_datasetSize = subDataset.size();
    if (m_datasetSize == 0)
        return;
    m_secondStageNetwork.train(subDataset, m_secondStageParams);
    vector<pair<double, double>> newDataset(maxKeyNum + 1, pair<double, double>{-1, -1});
    while (m_datasetSize > capacity)
        capacity /= density;
    capacity = capacity > maxKeyNum ? maxKeyNum : capacity;
    int cnt = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        if (subDataset[i].first != -1)
        {
            double p = m_secondStageNetwork.predict(subDataset[i].first);
            int maxIdx = max(capacity, m_datasetSize);
            int preIdx = static_cast<int>(p * (maxIdx - 1));
            insertData(newDataset, subDataset[i], preIdx, cnt);
        }
    }
    m_dataset = newDataset;
}

template <typename type>
pair<double, double> gappedNode<type>::find(double key)
{
    double p = m_secondStageNetwork.predict(key);
    int maxIdx = max(capacity, m_datasetSize);
    int preIdx = static_cast<int>(p * (maxIdx - 1));
    if (m_dataset[preIdx].first == key)
        return m_dataset[preIdx];
    else
    {
        int res = search(key, preIdx);
        if (res == -1)
            return {};
        return m_dataset[res];
    }
}

template <typename type>
bool gappedNode<type>::update(pair<double, double> data)
{
    double p = m_secondStageNetwork.predict(data.first);
    int maxIdx = max(capacity, m_datasetSize);
    int preIdx = static_cast<int>(p * (maxIdx - 1));
    if (m_dataset[preIdx].first == data.first)
    {
        m_dataset[preIdx].second = data.second;
        return true;
    }
    else
    {
        int res = search(data.first, preIdx);
        if (res == -1)
            return false;
        m_dataset[res].second = data.second;
        return true;
    }
}

template <typename type>
bool gappedNode<type>::del(double key)
{
    double p = m_secondStageNetwork.predict(key);
    int maxIdx = max(capacity, m_datasetSize);
    int preIdx = static_cast<int>(p * (maxIdx - 1));
    if (m_dataset[preIdx].first == key)
    {
        m_dataset[preIdx] = {-1, -1};
        m_datasetSize--;
        return true;
    }
    else
    {
        int res = search(key, preIdx);
        if (res == -1)
            return false;
        m_datasetSize--;
        m_dataset[res] = {-1, -1};
        return true;
    }
}

template <typename type>
bool gappedNode<type>::insert(pair<double, double> data)
{
    if (m_datasetSize * 1.0 / capacity >= density)
    {
        // If an additional insertion results in crossing the density
        // then we expand the gapped array
        expand();
    }

    double p = m_secondStageNetwork.predict(data.first);
    int maxIdx = max(capacity, m_datasetSize);
    int preIdx = static_cast<int>(p * (maxIdx - 1));

    // exponential search
    int start_idx, end_idx;
    while (m_dataset[preIdx].first == -1)
    {
        preIdx++;
        if (preIdx >= maxIdx)
            preIdx = 0;
    }
    if (m_dataset[preIdx].first > data.first)
    {
        int offset = 1;
        int i = preIdx;
        while (i >= 0 && m_dataset[i].first > data.first)
        {
            i = preIdx - offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = max(0, i);
        end_idx = preIdx - (offset / 4);
        while (m_dataset[end_idx].first == -1)
            end_idx++;
    }
    else
    {
        int offset = 1;
        int i = preIdx;
        while (i < maxIdx && m_dataset[i].first < data.first)
        {
            i = preIdx + offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = preIdx + (offset >> 2);
        end_idx = min(maxIdx, i);
    }

    // use binary search to find where to insert
    int cnt = 0;
    while (start_idx <= end_idx)
    {
        cnt = 0;
        for (int i = start_idx; i <= end_idx; i++)
        {
            if (m_dataset[i].first != -1)
                cnt++;
        }
        if (cnt <= 2)
            break;

        int mid = (start_idx + end_idx) >> 1;

        while (m_dataset[mid].first == -1 && mid < maxIdx - 1)
            mid++;

        if (mid == maxIdx - 1)
        {
            mid = (start_idx + end_idx) >> 1;
            for (; m_dataset[mid].first == -1 && mid > start_idx; mid--)
            {
            }
        }

        double tmp = m_dataset[mid].first;
        if (data.first < tmp)
        {
            if (end_idx == mid)
            {
                while (m_dataset[start_idx + 1].first == -1)
                    start_idx++;
                while (m_dataset[end_idx - 1].first == -1)
                    end_idx--;
            }
            else
                end_idx = mid;
        }
        else if (data.first > tmp)
        {
            start_idx = mid;
        }
    }
    // skip empty positions
    while (cnt == 2 && m_dataset[end_idx].first == -1)
        end_idx--;
    while (cnt == 2 && m_dataset[start_idx].first == -1)
        start_idx++;
    if (cnt == 2)
    {
        if (m_dataset[end_idx].first < data.first)
            end_idx++;
        else if (m_dataset[start_idx].first > data.first)
            end_idx = start_idx;
    }

    // if the insertion position is a gap,
    //  then we insert the element into the gap and are done
    if (end_idx > 0 && m_dataset[end_idx - 1].first == -1)
        m_dataset[end_idx - 1] = data;
    else
    {
        // If the insertion position is not a gap, we make
        // a gap at the insertion position by shifting the elements
        // by one position in the direction of the closest gap
        int j = end_idx + 1;
        while (j < maxIdx - 1 && m_dataset[j].first != -1)
            j++;
        if (j == maxIdx - 1)
        {
            j = end_idx - 1;
            while (j >= 0 && m_dataset[j].first != -1)
                j--;
            for (int i = j; i < end_idx - 1; i++)
                m_dataset[i] = m_dataset[i + 1];
            m_dataset[end_idx - 1] = data;
        }
        else
        {
            for (int i = j; i > end_idx; i--)
                m_dataset[i] = m_dataset[i - 1];
            m_dataset[end_idx] = data;
        }
    }
    m_datasetSize++;
    return true;
}

/*
search the index of key
0. Use p to get a approximate position of key
1. Exponential Search at first, and decide the range of binary search
2. Use Binary Search to search the gap array.
*/
template <typename type>
int gappedNode<type>::search(double key, int p)
{
    // exponential search
    int start_idx, end_idx;
    int maxIdx = max(capacity, m_datasetSize);
    while (m_dataset[p].first == -1)
    {
        p++;
        if (p >= maxIdx)
            p = 0;
    }
    if (m_dataset[p].first > key)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && m_dataset[i].first > key)
        {
            i = p - offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = max(0, i);
        end_idx = p - (offset / 4);
        while (m_dataset[end_idx].first == -1)
            end_idx++;
    }
    else
    {
        int offset = 1;
        int i = p;
        while (i < maxIdx && m_dataset[i].first < key)
        {
            i = p + offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = p + (offset >> 2);
        end_idx = min(maxIdx, i);
    }

    // binary search
    while (start_idx <= end_idx)
    {
        int mid = (start_idx + end_idx) >> 1;
        while (m_dataset[mid].first == -1 && mid < maxIdx - 1)
            mid++;
        if (mid == end_idx + 1)
        {
            mid--;
            while ((m_dataset[mid].first == -1) && (mid >= start_idx))
                mid--;
        }
        if (mid == maxIdx - 1)
        {
            mid = (start_idx + end_idx) >> 1;
            for (; m_dataset[mid].first == -1 && mid > start_idx; mid--)
            {
            };
        }
        if (start_idx == end_idx)
        {
            if (key == m_dataset[start_idx].first)
                return start_idx;
            else
                return -1;
        }
        double tmp = m_dataset[mid].first;
        if (key < tmp)
            end_idx = mid - 1;
        else if (key > tmp)
            start_idx = mid + 1;
        else
            return mid;
    }
    return -1;
}

// expand the vector when m_datasetSize / capacity >= density
template <typename type>
void gappedNode<type>::expand()
{
    int newSize = capacity / density;
    if (newSize > maxKeyNum)
    {
        capacity = maxKeyNum;
        return;
    }
    // retraining the node’s linear model on the existing keys
    m_secondStageNetwork.train(m_dataset, m_secondStageParams);

    // rescaling the model to predict positions in the expanded array
    capacity = newSize;

    // do model-based inserts of all the elements
    // in this node using the retrained RMI
    vector<pair<double, double>> newDataset(maxKeyNum + 1, pair<double, double>{-1, -1});
    int cnt = 0;
    for (int i = 0; i < maxKeyNum; i++)
    {
        if (m_dataset[i].first != -1)
        {
            double p = m_secondStageNetwork.predict(m_dataset[i].first);
            int maxIdx = max(capacity, m_datasetSize);
            int preIdx = static_cast<int>(p * (maxIdx - 1));
            insertData(newDataset, m_dataset[i], p, cnt);
        }
    }
    m_dataset = newDataset;
}

/*
vec: insert data into the new vector
idx: insertion position
cnt: current number of data in the vector
*/
template <typename type>
void gappedNode<type>::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt)
{
    cnt++;
    int maxIdx;
    if (capacity == cnt)
        maxIdx = capacity + 1;
    else
        maxIdx = max(capacity, cnt);
    while (vec[idx].first != -1 && idx < maxIdx)
    {
        idx++;
    }
    if (idx == maxIdx - 1)
    {
        int j = idx - 1;
        while (vec[j].first != -1)
            j--;
        for (; j < idx - 1; j++)
        {
            vec[j] = vec[j + 1];
        }
        idx--;
    }
    vec[idx] = data;
}

#endif