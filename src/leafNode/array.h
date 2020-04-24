#ifndef ARRAY_H
#define ARRAY_H

#include <math.h>
#include "leaf_node.h"
using namespace std;

class ArrayNode : public BasicLeafNode
{
public:
    ArrayNode(int maxInsertNumber, int threshold) : BasicLeafNode()
    {
        m_maxInsertNumber = maxInsertNumber;
        maxPositiveError = 0;
        maxNegativeError = 0;
    }

    void SetDataset(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key);
    bool Insert(pair<double, double> data);
    bool Delete(double key);
    bool Update(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset);

private:
    int m_maxInsertNumber; // the maximum number of Inserts
    int maxPositiveError;
    int maxNegativeError;
};

void ArrayNode::SetDataset(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;
    std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });

    model->Train(m_dataset);
    int maxError = 0;
    double p;
    for (int i = 0; i < m_datasetSize; i++)
    {
        p = model->Predict(m_dataset[i].first);
        // cout << "In array, Train: i:" << i << "\tkey:" << m_dataset[i].first << "\tp:" << p << "\tnew p:" << p * (m_datasetSize - 1) << endl;
        p *= m_datasetSize - 1;
        int error = i - p;
        if (error > maxPositiveError)
            maxPositiveError = error;
        if (error < maxNegativeError)
            maxNegativeError = error;
        if (error < 0)
            error = -error;
        if (error > maxError)
            maxError = error;
    }
    maxPositiveError++;
    maxNegativeError--;
}

pair<double, double> ArrayNode::Find(double key)
{
    // use learnedIndex to Find the data
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first == key)
    {
        return m_dataset[preIdx];
    }
    else
    {
        // binary search
        int start = max(0, preIdx + maxNegativeError);
        int end = min(m_datasetSize - 1, preIdx + maxPositiveError);

        int res = -1;
        while (start <= end)
        {
            int mid = (start + end) / 2;
            if (m_dataset[mid].first == key)
            {
                res = mid;
                break;
            }
            else if (start == end)
            {
                if (m_dataset[end].first == key)
                {
                    res = end;
                    break;
                }
                else
                    break;
            }
            else if (m_dataset[mid].first > key)
                end = mid;
            else
                start = mid + 1;
        }

        if (res != -1)
            return m_dataset[res];
        return {};
    }
}

bool ArrayNode::Insert(pair<double, double> data)
{
    // use learnedIndex to Find the data
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));

    // exponential search
    int start_idx;
    int end_idx;
    if (m_dataset[preIdx].first < data.first)
    {
        int i = 1;
        while (preIdx + i < m_datasetSize && m_dataset[preIdx + i].first < data.first)
            i *= 2;
        start_idx = preIdx + i / 2;
        end_idx = min(preIdx + i, m_datasetSize - 1);
    }
    else
    {
        int i = 1;
        while (preIdx - i >= 0 && m_dataset[preIdx - i].first >= data.first)
            i *= 2;
        start_idx = max(0, preIdx - i);
        end_idx = preIdx - i / 2;
    }
    end_idx = min(end_idx, m_datasetSize - 1);

    // binary search
    while (start_idx <= end_idx)
    {
        int mid = (start_idx + end_idx) / 2;
        if (mid == 0 && m_dataset[mid].first >= data.first)
        {
            preIdx = mid;
            break;
        }
        if (mid == m_dataset.size() - 1 && m_dataset[mid].first < data.first)
        {
            m_dataset.push_back(data);
            m_datasetSize++;
            return true;
        }
        if (m_dataset[mid - 1].first < data.first && m_dataset[mid].first >= data.first)
        {
            preIdx = mid;
            break;
        }
        if (m_dataset[mid].first > data.first)
            end_idx = mid - 1;
        else
            start_idx = mid + 1;
    }

    // Insert data
    m_dataset.push_back(m_dataset[m_datasetSize - 1]);
    m_datasetSize++;
    for (int i = m_datasetSize - 2; i > preIdx; i--)
        m_dataset[i] = m_dataset[i - 1];
    m_dataset[preIdx] = data;

    // updata error
    int pre = static_cast<int>(p * (m_datasetSize - 1));
    int error = preIdx - pre;
    if (error > maxPositiveError)
    {
        maxPositiveError = error;
        maxPositiveError++;
    }
    if (error < maxNegativeError)
    {
        maxNegativeError = error;
        maxNegativeError--;
    }

    // If the current number is greater than the maximum,
    // the child node needs to be reTrained
    if (m_datasetSize >= m_maxInsertNumber)
        SetDataset(m_dataset);
    return true;
}

bool ArrayNode::Delete(double key)
{
    // use learnedIndex to Find the data
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first != key)
    {
        // binary search
        int start = max(0, preIdx + maxNegativeError);
        int end = min(m_datasetSize - 1, preIdx + maxPositiveError);

        int res = -1;
        while (start <= end)
        {
            int mid = (start + end) / 2;
            if (m_dataset[mid].first == key)
            {
                res = mid;
                break;
            }
            else if (start == end)
            {
                if (m_dataset[end].first == key)
                {
                    res = end;
                    break;
                }
                else
                    break;
            }
            else if (m_dataset[mid].first > key)
                end = mid;
            else
                start = mid + 1;
        }

        if (res != -1)
            preIdx = res;
        else
            return false;
    }
    for (int i = preIdx; i < m_datasetSize - 1; i++)
        m_dataset[i] = m_dataset[i + 1];
    m_datasetSize--;
    m_dataset.pop_back();
    return true;
}

bool ArrayNode::Update(pair<double, double> data)
{
    // use learnedIndex to Find the data
    double p = model->Predict(data.first);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));
    if (m_dataset[preIdx].first != data.first)
    {
        // binary search
        int start = max(0, preIdx + maxNegativeError);
        int end = min(m_datasetSize - 1, preIdx + maxPositiveError);

        int res = -1;
        while (start <= end)
        {
            int mid = (start + end) / 2;
            if (m_dataset[mid].first == data.first)
            {
                res = mid;
                break;
            }
            else if (start == end)
            {
                if (m_dataset[end].first == data.first)
                {
                    res = end;
                    break;
                }
                else
                    break;
            }
            else if (m_dataset[mid].first > data.first)
                end = mid;
            else
                start = mid + 1;
        }

        if (res != -1)
            preIdx = res;
        else
            return false;
    }
    m_dataset[preIdx].second = data.second;
    return true;
}

long double ArrayNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset)
{
    int datasetSize = dataset.size();
    if (datasetSize == 0)
        return 0;

    // calculate cost
    long double totalCost = 0;
    double READCOST = 1.2;
    double WRITECOST = 3.5;
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