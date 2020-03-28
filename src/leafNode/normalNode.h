#ifndef NORMAL_NODE_H
#define NORMAL_NODE_H

#include "../params.h"
#include "../../cpp-btree/btree_map.h"
#include <array>

template <typename type>
class normalNode
{
public:
    normalNode(int threshold, params secondStageParams, int maxInsertNumber)
    {
        m_threshold = threshold;
        m_maxInsertNumber = maxInsertNumber;
        isUseTree = false;

        maxPositiveError = 0;
        maxNegativeError = 0;

        m_secondStageParams = secondStageParams;

        isLeafNode = true;
    }

    void train(const vector<pair<double, double>> &subDataset);
    int getSize() { return m_datasetSize; }
    bool isLeaf() { return isLeafNode; }
    void getDataset(vector<pair<double, double>> &dataset)
    {
        for (int i = 0; i < m_subDataset.size(); i++)
        {
            dataset.push_back(m_subDataset[i]);
        }
    }

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    int predict(double key)
    {
        double p = m_secondStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        int index = int(m_subDataset[preIdx].second);
        return index;
    }

private:
    int m_maxInsertNumber; // the maximum number of inserts
    int m_threshold;       // the maximum error margin
    int maxPositiveError;
    int maxNegativeError;
    bool isUseTree; // used to indicate whether to use b-tree

    int m_datasetSize;
    vector<pair<double, double>> m_subDataset; // <key, value>

    params m_secondStageParams; // parameters of network
    type m_secondStageNetwork = type();

    btree::btree_map<double, int> m_tree; // <key, index in subDataset>

    bool isLeafNode;
};

template <typename type>
void normalNode<type>::train(const vector<pair<double, double>> &subDataset)
{
    m_subDataset = subDataset;
    m_datasetSize = m_subDataset.size();
    if (m_datasetSize == 0)
        return;
    std::sort(m_subDataset.begin(), m_subDataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    m_secondStageNetwork.train(m_subDataset, m_secondStageParams);
    double maxError = 0;
    double p;
    for (int i = 0; i < m_datasetSize; i++)
    {
        p = m_secondStageNetwork.predict(m_subDataset[i].first);
        p *= m_datasetSize - 1;
        int error = i - p;
        if (error > maxPositiveError)
            maxPositiveError = error;
        if (error < maxNegativeError)
            maxNegativeError = error;
        if (abs(error) > maxError)
            maxError = abs(error);
    }
    maxPositiveError++;
    maxNegativeError--;
    if (maxError > m_threshold)
    {
        isUseTree = true;
        for (int i = 0; i < m_datasetSize; i++)
            m_tree.insert({m_subDataset[i].first, i});
    }
    else
        isUseTree = false;
}

template <typename type>
pair<double, double> normalNode<type>::find(double key)
{
    //  use B-Tree if the data is particularly hard to learn
    if (this->isUseTree)
    {
        auto result = m_tree.find(key); // result:{key, index}
        if (result != m_tree.end())
            return m_subDataset[int(result->second)];
        else
            return {};
    }
    else
    {
        // use learnedIndex to find the data
        double p = m_secondStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        if (m_subDataset[preIdx].first == key)
        {
            return m_subDataset[preIdx];
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
                if (m_subDataset[mid].first == key)
                {
                    res = mid;
                    break;
                }
                else if (start == end)
                {
                    if (m_subDataset[end].first == key)
                    {
                        res = end;
                        break;
                    }
                    else
                        break;
                }
                else if (m_subDataset[mid].first > key)
                    end = mid;
                else
                    start = mid + 1;
            }

            if (res != -1)
                return m_subDataset[res];
            else
            {
                start = 0;
                end = m_subDataset.size() - 1;
                while (start <= end)
                {
                    int mid = (start + end) / 2;
                    if (m_subDataset[mid].first == key)
                    {
                        res = mid;
                        break;
                    }
                    else if (start == end)
                    {
                        if (m_subDataset[end].first == key)
                        {
                            res = end;
                            break;
                        }
                        else
                            break;
                    }
                    else if (m_subDataset[mid].first > key)
                        end = mid;
                    else
                        start = mid + 1;
                }
                if (res != -1)
                    return m_subDataset[res];
            }
            return {};
        }
    }
}

template <typename type>
bool normalNode<type>::insert(pair<double, double> data)
{
    // use learnedIndex to find the data
    double p = m_secondStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (m_datasetSize - 1));

    // exponential search
    int start_idx;
    int end_idx;
    if (m_subDataset[preIdx].first < data.first)
    {
        int i = 1;
        while (preIdx + i < m_datasetSize && m_subDataset[preIdx + i].first < data.first)
            i *= 2;
        start_idx = preIdx + i / 2;
        end_idx = min(preIdx + i, m_datasetSize - 1);
    }
    else
    {
        int i = 1;
        while (preIdx - i >= 0 && m_subDataset[preIdx - i].first >= data.first)
            i *= 2;
        start_idx = max(0, preIdx - i);
        end_idx = preIdx - i / 2;
    }
    end_idx = min(end_idx, m_datasetSize - 1);

    // binary search
    while (start_idx <= end_idx)
    {
        int mid = (start_idx + end_idx) / 2;
        if (mid == 0 && m_subDataset[mid].first >= data.first)
        {
            preIdx = mid;
            break;
        }
        if (mid == m_subDataset.size() - 1 && m_subDataset[mid].first < data.first)
        {
            if (isUseTree)
                m_tree.insert({data.first, mid + 1});
            m_subDataset.push_back(data);
            m_datasetSize++;
            return true;
        }
        if (m_subDataset[mid - 1].first < data.first && m_subDataset[mid].first >= data.first)
        {
            preIdx = mid;
            break;
        }
        if (m_subDataset[mid].first > data.first)
            end_idx = mid - 1;
        else
            start_idx = mid + 1;
    }

    if (isUseTree)
        m_tree.insert({data.first, preIdx});

    // insert data
    m_subDataset.push_back(m_subDataset[m_datasetSize - 1]);
    m_datasetSize++;
    for (int i = m_datasetSize - 2; i > preIdx; i--)
        m_subDataset[i] = m_subDataset[i - 1];
    m_subDataset[preIdx] = data;

    // If the current number is greater than the maximum,
    // the child node needs to be retrained
    if (m_datasetSize >= m_maxInsertNumber)
        train(m_subDataset);
    return true;
}

template <typename type>
bool normalNode<type>::del(double key)
{
    //  use B-Tree if the data is particularly hard to learn
    if (isUseTree)
    {
        auto result = m_tree.find(key); // result:{key, index}
        if (result != m_tree.end())
        {
            int idx = int(result->second);
            for (int i = idx; i < m_datasetSize - 1; i++)
                m_subDataset[i] = m_subDataset[i + 1];
            m_datasetSize--;
            m_subDataset.pop_back();
            m_tree.erase(key);
            return true;
        }
        else
            return false;
    }
    else
    {
        // use learnedIndex to find the data
        double p = m_secondStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        if (m_subDataset[preIdx].first != key)
        {
            // binary search
            int start = max(0, preIdx + maxNegativeError);
            int end = min(m_datasetSize - 1, preIdx + maxPositiveError);

            int res = -1;
            while (start <= end)
            {
                int mid = (start + end) / 2;
                if (m_subDataset[mid].first == key)
                {
                    res = mid;
                    break;
                }
                else if (start == end)
                {
                    if (m_subDataset[end].first == key)
                    {
                        res = end;
                        break;
                    }
                    else
                        break;
                }
                else if (m_subDataset[mid].first > key)
                    end = mid;
                else
                    start = mid + 1;
            }

            if (res != -1)
                preIdx = res;
            else
            {
                start = 0;
                end = m_subDataset.size() - 1;
                while (start <= end)
                {
                    int mid = (start + end) / 2;
                    if (m_subDataset[mid].first == key)
                    {
                        res = mid;
                        break;
                    }
                    else if (start == end)
                    {
                        if (m_subDataset[end].first == key)
                        {
                            res = end;
                            break;
                        }
                        else
                            break;
                    }
                    else if (m_subDataset[mid].first > key)
                        end = mid;
                    else
                        start = mid + 1;
                }
                if (res != -1)
                    preIdx = res;
                else
                    return false;
            }
        }
        for (int i = preIdx; i < m_datasetSize - 1; i++)
            m_subDataset[i] = m_subDataset[i + 1];
        m_datasetSize--;
        m_subDataset.pop_back();
        return true;
    }
}

template <typename type>
bool normalNode<type>::update(pair<double, double> data)
{
    //  use B-Tree if the data is particularly hard to learn
    if (isUseTree)
    {
        auto result = m_tree.find(data.first); // result:{key, index}
        if (result != m_tree.end())
        {
            m_subDataset[int(result->second)].second = data.second;
            return true;
        }
        else
            return false;
    }
    else
    {
        // use learnedIndex to find the data
        double p = m_secondStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        if (m_subDataset[preIdx].first != data.first)
        {
            // binary search
            int start = max(0, preIdx + maxNegativeError);
            int end = min(m_datasetSize - 1, preIdx + maxPositiveError);

            int res = -1;
            while (start <= end)
            {
                int mid = (start + end) / 2;
                if (m_subDataset[mid].first == data.first)
                {
                    res = mid;
                    break;
                }
                else if (start == end)
                {
                    if (m_subDataset[end].first == data.first)
                    {
                        res = end;
                        break;
                    }
                    else
                        break;
                }
                else if (m_subDataset[mid].first > data.first)
                    end = mid;
                else
                    start = mid + 1;
            }

            if (res != -1)
                preIdx = res;
            else
            {
                start = 0;
                end = m_subDataset.size() - 1;
                while (start <= end)
                {
                    int mid = (start + end) / 2;
                    if (m_subDataset[mid].first == data.first)
                    {
                        res = mid;
                        break;
                    }
                    else if (start == end)
                    {
                        if (m_subDataset[end].first == data.first)
                        {
                            res = end;
                            break;
                        }
                        else
                            break;
                    }
                    else if (m_subDataset[mid].first > data.first)
                        end = mid;
                    else
                        start = mid + 1;
                }
                if (res == -1)
                    return false;
                preIdx = res;
            }
        }
        m_subDataset[preIdx].second = data.second;
        return true;
    }
}

#endif