#ifndef GAPPED_NODE_H
#define GAPPED_NODE_H

#include "../params.h"
#include "../../cpp-btree/btree_map.h"
#include <array>
#include "../node.h"

static const int childNumber = 20;

template <typename type>
class gappedNode : public node
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
        maxIndex = 0;
    }

    void train(const vector<pair<double, double>> &subDataset);
    int getSize() { return m_datasetSize; }
    bool isLeaf() { return isLeafNode; }
    void getDataset(vector<pair<double, double>> &dataset)
    {
        for (int i = 0; i < m_dataset.size(); i++)
        {
            if (m_dataset[i].first != -1)
                dataset.push_back(m_dataset[i]);
        }
    }
    long double getCost(btree::btree_map<double, pair<int, int>> cntTree, int childNum, vector<pair<double, double>> &dataset);

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
    int maxIndex; // tht index of the last one

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
    {
        m_dataset = vector<pair<double, double>>(maxKeyNum, pair<double, double>{-1, -1});
        return;
    }
    m_secondStageNetwork.train(subDataset, m_secondStageParams);
    vector<pair<double, double>> newDataset(maxKeyNum, pair<double, double>{-1, -1});
    while (m_datasetSize > capacity)
        capacity /= density;
    capacity = capacity > maxKeyNum ? maxKeyNum : capacity;
    for (int i = 0; i < m_datasetSize; i++)
    {
        if (subDataset[i].first != -1)
        {
            double p = m_secondStageNetwork.predict(subDataset[i].first);
            int preIdx = static_cast<int>(p * (capacity - 1));
            insertData(newDataset, subDataset[i], preIdx, maxIndex);
        }
    }
    m_dataset = newDataset;
    m_secondStageNetwork.train(m_dataset, m_secondStageParams);
}

template <typename type>
pair<double, double> gappedNode<type>::find(double key)
{
    double p = m_secondStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (capacity - 1));
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
    int preIdx = static_cast<int>(p * (capacity - 1));
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
    int preIdx = static_cast<int>(p * (capacity - 1));
    if (m_dataset[preIdx].first == key)
    {
        m_dataset[preIdx] = {-1, -1};
        m_datasetSize--;
    }
    else
    {
        int res = search(key, preIdx);
        if (res == -1)
            return false;
        m_datasetSize--;
        m_dataset[res] = {-1, -1};
        preIdx = res;
    }
    if (preIdx == maxIndex)
        maxIndex--;
    else
    {
        bool isMove = false;
        if ((preIdx > 0 && m_dataset[preIdx - 1].first == -1) || (m_dataset[preIdx + 1].first == -1))
        {
            int i = preIdx + 1;
            if (m_dataset[preIdx - 1].first == -1)
                i++;
            while ((i + 1) <= maxIndex)
            {
                m_dataset[i] = m_dataset[i + 1];
                if (m_dataset[i + 1].first != -1 && m_dataset[i + 2].first != -1)
                {
                    m_dataset[i + 1] = {-1, -1};
                    break;
                }
                i++;
            }
            if ((i + 1) == maxIndex + 1)
            {
                m_dataset[i] = {-1, -1};
                maxIndex--;
            }
        }
    }
}

template <typename type>
bool gappedNode<type>::insert(pair<double, double> data)
{
    if ((capacity < maxKeyNum) && (m_datasetSize * 1.0 / capacity >= density))
    {
        // If an additional insertion results in crossing the density
        // then we expand the gapped array
        expand();
    }

    double p = m_secondStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (capacity - 1));
    if (m_datasetSize == 0)
    {
        m_dataset[preIdx] = data;
        m_datasetSize++;
        maxIndex = 0;
        return true;
    }

    int start_idx = 0, end_idx = maxIndex;
    bool isViolative = false;
    for (int i = preIdx - 1; i >= 0; i--)
    {
        if (m_dataset[i].first != -1 && m_dataset[i].first > data.first)
        {
            end_idx = i;
            isViolative = true;
            break;
        }
    }
    for (int i = preIdx; i <= maxIndex; i++)
    {
        if (m_dataset[i].first != -1 && m_dataset[i].first < data.first)
        {
            start_idx = i;
            isViolative = true;
            break;
        }
    }
    if (isViolative)
    {
        if (m_dataset[0].first != -1 && m_dataset[0].first > data.first)
            preIdx = 0;
        else if (m_dataset[0].first == -1 && m_dataset[1].first != -1 && m_dataset[1].first > data.first)
            preIdx = 1;
        else if (m_dataset[maxIndex].first < data.first)
            preIdx = maxIndex + 1;
        else
        {
            // exponential search
            int offset = 1;
            int i = start_idx;
            while (i <= end_idx && m_dataset[i].first < data.first)
            {
                i = start_idx + offset;
                offset = offset << 1;
                if (m_dataset[i].first == -1)
                    i++;
            }
            start_idx = (start_idx + offset >> 2) > maxIndex ? maxIndex : (start_idx + offset >> 2);
            end_idx = min(i, maxIndex);
            if (m_dataset[start_idx].first == -1)
                start_idx++;

            // use binary search to find where to insert
            while (start_idx < end_idx)
            {
                int mid = (start_idx + end_idx) >> 1;
                if (m_dataset[mid].first == -1)
                {
                    int left = max(start_idx, mid - 1);
                    int right = min(end_idx, mid + 1);
                    if (m_dataset[left].first < data.first && m_dataset[right].first > data.first)
                    {
                        preIdx = mid;
                        break;
                    }
                    else if (m_dataset[left].first > data.first)
                        end_idx = left;
                    else if (m_dataset[right].first < data.first)
                        start_idx = right;
                }
                else
                {
                    int left = (m_dataset[mid - 1].first == -1) ? mid - 2 : mid - 1;
                    left = max(start_idx, left);
                    int right = min(end_idx, mid);
                    if (left == right)
                        right++;
                    if (m_dataset[right].first == -1)
                        right++;
                    if (m_dataset[left].first < data.first && m_dataset[right].first > data.first)
                    {
                        preIdx = right;
                        break;
                    }
                    else if (m_dataset[left].first > data.first)
                        end_idx = left;
                    else if (m_dataset[mid].first < data.first)
                        start_idx = mid;
                }
                if (m_dataset[start_idx].first == -1)
                    start_idx--;
                if (m_dataset[end_idx].first == -1)
                    end_idx++;
            }
            if (start_idx == end_idx)
                preIdx = end_idx;
        }
    }

    // if the insertion position is a gap,
    //  then we insert the element into the gap and are done
    if (m_dataset[preIdx].first == -1)
    {
        int empty = 0;
        int i;
        for (i = preIdx - 1; i >= 0; i--)
        {
            if (m_dataset[i].first == -1)
                empty++;
            else
            {
                break;
            }
        }
        if (empty > 1)
        {
            m_dataset[i + 2] = data;
            m_datasetSize++;
            maxIndex = max(maxIndex, i + 2);
            return true;
        }
        if (i == -1)
            preIdx = 0;
        m_dataset[preIdx] = data;
        m_datasetSize++;
        maxIndex = max(maxIndex, preIdx);
        return true;
    }
    else
    {
        // If the insertion position is not a gap, we make
        // a gap at the insertion position by shifting the elements
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
    while (m_dataset[p].first == -1)
    {
        p++;
        if (p > maxIndex)
            p = 0;
    }
    if (m_dataset[p].first > key)
    {
        int offset = 1;
        int i = p;
        while (i >= 0 && m_dataset[i].first > key)
        {
            if (offset == 0)
                return -1;
            i = p - offset;
            offset = offset << 1;
            i = max(0, i);
            if (m_dataset[i].first == -1)
                i++;
        }
        start_idx = i;
        end_idx = (p - (offset >> 2)) < 0 ? 0 : p - (offset >> 2);
        if (m_dataset[end_idx].first == -1)
            end_idx++;
    }
    else if (m_dataset[p].first < key)
    {
        int offset = 1;
        int i = p;
        while (i <= maxIndex && m_dataset[i].first < key)
        {
            i = p + offset;
            offset = offset << 1;
            i = min(i, maxIndex);
            if (m_dataset[i].first == -1)
                i++;
        }
        start_idx = (p + offset >> 2) > maxIndex ? maxIndex : (p + offset >> 2);
        end_idx = i;
        if (m_dataset[start_idx].first == -1)
            start_idx++;
    }
    else
    {
        return p;
    }

    // binary search
    while (start_idx <= end_idx)
    {
        int mid = (start_idx + end_idx) >> 1;
        if (m_dataset[mid].first == -1)
            mid++;
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
    if (capacity == maxKeyNum)
    {
        m_secondStageNetwork.train(m_dataset, m_secondStageParams);
        return;
    }
    int newSize = capacity / density;
    while (m_datasetSize * 1.0 / newSize >= density)
        newSize /= density;
    if (newSize > maxKeyNum)
    {
        newSize = maxKeyNum;
    }
    // retraining the node's linear model on the existing keys
    m_secondStageNetwork.train(m_dataset, m_secondStageParams);

    // rescaling the model to predict positions in the expanded array
    capacity = newSize;

    // do model-based inserts of all the elements
    // in this node using the retrained RMI
    vector<pair<double, double>> newDataset(maxKeyNum, pair<double, double>{-1, -1});
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
    maxIndex = cnt;
    m_dataset = newDataset;
}

/*
vec: insert data into the new vector
idx: insertion position
cnt: current maxIndex of data in the vector
*/
template <typename type>
void gappedNode<type>::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt)
{
    if (idx < cnt)
        idx = cnt + 1;
    while (vec[idx].first != -1 && idx < capacity)
    {
        idx++;
    }
    if (idx == capacity)
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
    int empty = 0;
    int i;
    for (i = idx - 1; i >= 0; i--)
    {
        if (vec[i].first == -1)
            empty++;
        else
        {
            break;
        }
    }
    if (empty > 1)
    {
        vec[i + 2] = data;
        cnt = i + 2;
        return;
    }
    if (i == -1)
    {
        vec[0] = data;
        cnt = 0;
    }
    else
    {
        vec[idx] = data;
        cnt = idx;
    }
}

template <typename type>
long double gappedNode<type>::getCost(btree::btree_map<double, pair<int, int>> cntTree, int childNum, vector<pair<double, double>> &dataset)
{
    m_datasetSize = dataset.size();
    if (m_datasetSize == 0)
    {
        m_dataset = vector<pair<double, double>>(maxKeyNum, pair<double, double>{-1, -1});
        return 0;
    }
    m_secondStageNetwork.train(dataset, m_secondStageParams);
    vector<pair<double, double>> newDataset(maxKeyNum, pair<double, double>{-1, -1});
    while (m_datasetSize > capacity)
        capacity /= density;
    capacity = capacity > maxKeyNum ? maxKeyNum : capacity;
    for (int i = 0; i < m_datasetSize; i++)
    {
        double p = m_secondStageNetwork.predict(dataset[i].first);
        int preIdx = static_cast<int>(p * (capacity - 1));
        insertData(newDataset, dataset[i], preIdx, maxIndex);
    }
    m_dataset = newDataset;
    m_secondStageNetwork.train(m_dataset, m_secondStageParams);

    // calculate cost
    long double totalCost = 0;
    double READCOST = 1.2;
    double WRITECOST = 3.5;
    for (int i = 0; i < m_datasetSize; i++)
    {
        pair<int, int> tmp = cntTree.find(dataset[i].first);
        double tmpRead = tmp.first * READCOST;
        double tmpWrite = tmp.first * WRITECOST;
        totalCost += tmpRead + tmpWrite;
    }
    return totalCost;
}

#endif