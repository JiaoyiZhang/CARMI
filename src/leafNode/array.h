#ifndef ARRAY_H
#define ARRAY_H

#include <math.h>
#include "leafNode.h"
using namespace std;

class arrayNode : public basicLeafNode
{
public:
    arrayNode(int maxInsertNumber, params p, int threshold) : basicLeafNode(p)
    {
        m_threshold = threshold;
        m_maxInsertNumber = maxInsertNumber;
        isUseTree = false;
        maxPositiveError = 0;
        maxNegativeError = 0;
    }

    void train(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset);

private:
    int m_maxInsertNumber; // the maximum number of inserts
    int m_threshold;       // the maximum error margin
    int maxPositiveError;
    int maxNegativeError;

    bool isUseTree;                       // used to indicate whether to use b-tree
    btree::btree_map<double, int> m_tree; // <key, index in subDataset>
};

void arrayNode::train(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;
    std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });

    model->train(m_dataset, parameter);
    int maxError = 0;
    double p;
    for (int i = 0; i < m_datasetSize; i++)
    {
        p = model->predict(m_dataset[i].first);
        // cout << "In array, train: i:" << i << "\tkey:" << m_dataset[i].first << "\tp:" << p << "\tnew p:" << p * (m_datasetSize - 1) << endl;
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
    if (maxError > m_threshold)
    {
        isUseTree = true;
        for (int i = 0; i < m_datasetSize; i++)
            m_tree.insert({m_dataset[i].first, i});
    }
    else
        isUseTree = false;
}

pair<double, double> arrayNode::find(double key)
{
    //  use B-Tree if the data is particularly hard to learn
    if (this->isUseTree)
    {
        auto result = m_tree.find(key); // result:{key, index}
        if (result != m_tree.end())
            return m_dataset[int(result->second)];
        else
            return {};
    }
    else
    {
        // use learnedIndex to find the data
        double p = model->predict(key);
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
}

bool arrayNode::insert(pair<double, double> data)
{
    // use learnedIndex to find the data
    double p = model->predict(data.first);
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
            if (isUseTree)
                m_tree.insert({data.first, mid + 1});
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

    if (isUseTree)
        m_tree.insert({data.first, preIdx});

    // insert data
    m_dataset.push_back(m_dataset[m_datasetSize - 1]);
    m_datasetSize++;
    for (int i = m_datasetSize - 2; i > preIdx; i--)
        m_dataset[i] = m_dataset[i - 1];
    m_dataset[preIdx] = data;

    // updata error
    if (isUseTree == false)
    {
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
        if (error < 0)
            error = -error;
        if (error > m_threshold)
        {
            isUseTree = true;
            for (int i = 0; i < m_datasetSize; i++)
                m_tree.insert({m_dataset[i].first, i});
        }
    }

    // If the current number is greater than the maximum,
    // the child node needs to be retrained
    if (m_datasetSize >= m_maxInsertNumber)
        train(m_dataset);
    return true;
}

bool arrayNode::del(double key)
{
    //  use B-Tree if the data is particularly hard to learn
    if (isUseTree)
    {
        auto result = m_tree.find(key); // result:{key, index}
        if (result != m_tree.end())
        {
            int idx = int(result->second);
            for (int i = idx; i < m_datasetSize - 1; i++)
                m_dataset[i] = m_dataset[i + 1];
            m_datasetSize--;
            m_dataset.pop_back();
            m_tree.erase(key);
            return true;
        }
        else
            return false;
    }
    else
    {
        // use learnedIndex to find the data
        double p = model->predict(key);
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
}

bool arrayNode::update(pair<double, double> data)
{
    //  use B-Tree if the data is particularly hard to learn
    if (isUseTree)
    {
        auto result = m_tree.find(data.first); // result:{key, index}
        if (result != m_tree.end())
        {
            m_dataset[int(result->second)].second = data.second;
            return true;
        }
        else
            return false;
    }
    else
    {
        // use learnedIndex to find the data
        double p = model->predict(data.first);
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
}

long double arrayNode::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, vector<pair<double, double>> &dataset)
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