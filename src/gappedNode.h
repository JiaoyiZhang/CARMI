#ifndef GAPPED_NODE_H
#define GAPPED_NODE_H

#include "params.h"
#include "../cpp-btree/btree_map.h"
#include "nn.h"
#include <array>

static const int childNumber = 20;

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
        vector<pair<double, double>> tmp(3096, {-1, -1});
        m_dataset = tmp;
    }

    void train(const vector<pair<double, double>> &subDataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool update(pair<double, double> data);
    bool del(double key);

private:
    int search(double key, int p);                                                                    // return the index of key
    bool insertLeaf(pair<double, double> data, int idx);                                              // insert data into leaf nodes
    void insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt); // insert data into new vector

    void expand(); // only for leaf node
    void split();  // only for leaf node

private:
    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    params m_secondStageParams; // parameters of network
    net m_secondStageNetwork = net();

    bool isLeafNode;
    vector<gappedNode *> children; // If the current node is not a leaf node, there are two child nodes

    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data
};

void gappedNode::train(const vector<pair<double, double>> &subDataset)
{
    m_dataset = subDataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;
    std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    m_secondStageNetwork.insert(m_dataset, m_secondStageParams.maxEpoch, m_secondStageParams.learningRate, m_secondStageParams.neuronNumber);
    m_secondStageNetwork.train();

    if (m_datasetSize >= maxKeyNum)
        split();
}

pair<double, double> gappedNode::find(double key)
{
    double p = m_secondStageNetwork.predict(key);
    if (!isLeafNode)
    {
        int preIdx = int(p * (childNumber - 1));
        return children[preIdx]->find(key);
    }
    else
    {
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
}

bool gappedNode::update(pair<double, double> data)
{
    double p = m_secondStageNetwork.predict(data.first);
    if (!isLeafNode)
    {
        int preIdx = int(p * (childNumber - 1));
        return children[preIdx]->update(data);
    }
    else
    {
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
}

bool gappedNode::del(double key)
{
    double p = m_secondStageNetwork.predict(key);
    if (!isLeafNode)
    {
        int preIdx = int(p * (childNumber - 1));
        return children[preIdx]->del(key);
    }
    else
    {
        int maxIdx = max(capacity, m_datasetSize);
        int preIdx = static_cast<int>(p * (maxIdx - 1));
        if (m_dataset[preIdx].first == key)
        {
            m_dataset[preIdx] = {-1, -1};
            return true;
        }
        else
        {
            int res = search(key, preIdx);
            if (res == -1)
                return false;
            m_dataset[res] = {-1, -1};
            return true;
        }
    }
}

bool gappedNode::insert(pair<double, double> data)
{
    double p = m_secondStageNetwork.predict(data.first);
    if (!isLeafNode)
    {
        int preIdx = int(p * (childNumber - 1));
        return children[preIdx]->insert(data);
    }
    else
    {
        int maxIdx = max(capacity, m_datasetSize);
        int preIdx = static_cast<int>(p * (maxIdx - 1));
        if (m_datasetSize >= maxKeyNum)
        {
            // if an insert will push a leaf node’s
            // data structure over its maximum bound number of keys,
            // then we split the leaf data node
            split();
            if (p < 0.5)
                return children[0]->insert(data);
            else
                return children[1]->insert(data);
        }
        else if (m_datasetSize * 1.0 / capacity >= density)
        {
            // If an additional insertion results in crossing the density
            // then we expand the gapped array
            expand();
        }
        return insertLeaf(data, p);
    }
}

/*
search the index of key
0. Use p to get a approximate position of key
1. Exponential Search at first, and decide the range of binary search
2. Use Binary Search to search the gap array.
*/
int gappedNode::search(double key, int p)
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
        if(start_idx == end_idx)
        {
            if(key == m_dataset[start_idx].first)
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

// called in insert()
bool gappedNode::insertLeaf(pair<double, double> data, int p)
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
    if (m_dataset[p].first > data.first)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && m_dataset[i].first > data.first)
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
        while (i < maxIdx && m_dataset[i].first < data.first)
        {
            i = p + offset;
            offset = offset << 1;
            while (m_dataset[i].first == -1)
                i++;
        }
        start_idx = p + (offset >> 2);
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
The function is called in expand() and split().

vec: insert data into the new vector
idx: insertion position
cnt: current number of data in the vector
*/
void gappedNode::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt)
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

// expand the vector when m_datasetSize / capacity >= density
void gappedNode::expand()
{
    int newSize = capacity / density;
    if (newSize > maxKeyNum)
    {
        capacity = maxKeyNum;
        return;
    }
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
    if (m_datasetSize * 1.0 / capacity >= density)
        expand();
}

// split this model when m_datasetSize >= maxKeyNum
void gappedNode::split()
{
    isLeafNode = false;
    vector<double> rate;
    double r = 1.0 / childNumber;
    for (int i = 0; i < childNumber; i++)
    {
        rate.push_back(r * i);

        gappedNode *tmp = new gappedNode(maxKeyNum, m_secondStageParams, capacity);
        children.push_back(tmp);
    }
    int idx = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        if (m_dataset[idx].first == -1)
            i--;
        else
        {
            double p = m_secondStageNetwork.predict(m_dataset[i].first);
            int preIdx = int(p * (childNumber - 1));
            insertData(children[preIdx]->m_dataset, m_dataset[idx], (p - rate[preIdx]) * (capacity - 1), children[preIdx]->m_datasetSize);
        }
        idx++;
    }
    for (int i = 0; i < childNumber; i++)
    {
        children[i]->train(children[i]->m_dataset);
    }
}

#endif