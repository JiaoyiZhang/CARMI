#ifndef LEARNED_INDEX_H
#define LEARNED_INDEX_H

#include "nn.h"
#include "../cpp-btree/btree_map.h"
#include <array>
class params
{
public:
    int maxEpoch;
    double learningRate;
    int neuronNumber;
};

class secondStageNode
{
public:
    secondStageNode(int threshold, params secondStageParams)
    {
        m_threshold = threshold;
        isUseTree = false;
        isValidNode = false;
        maxNegativeError = 0;
        maxPositiveError = 0;

        m_secondStageParams = secondStageParams;
    }

    bool useTree()
    {
        return isUseTree;
    }

    bool validNode()
    {
        return isValidNode;
    }

    int getPositive()
    {
        return maxPositiveError;
    }

    int getNagetive()
    {
        return maxNegativeError;
    }

    pair<double, double> treeFind(double key)
    {
        assert(isUseTree && "Called treeFind but the tree isn't supposed to be used");
        auto result = m_tree.find(key);
        if (result != m_tree.end())
            return pair<double, double>(result->first, result->second); // return the result of btree.find
        else
            return pair<double, double>(-1, -1); // if no find, return <-1,-1>
    }

    void train(const vector<pair<double, double>> &subDataset);

    // return the index in totalDataset
    int predict(double key)
    {
        double p = m_secondStageNetwork.predict(key); // return the index in subDataset
        int preIdx = static_cast<int>(p * (m_datasetSize-1));
        // preIdx = max(0, preIdx);
        // preIdx = min(m_datasetSize, preIdx);
        int index = int(m_subDataset[preIdx].second);
        return index;
    }

private:
    int m_threshold;
    bool isUseTree;
    bool isValidNode;

    int maxNegativeError;
    int maxPositiveError;

    int m_datasetSize;
    vector<pair<double, double>> m_subDataset; // <key, index in totalDataset>

    params m_secondStageParams;
    net m_secondStageNetwork = net();

    btree::btree_map<double, int> m_tree; // <key, index in totalDataset>
};

void secondStageNode::train(const vector<pair<double, double>> &subDataset)
{
    m_subDataset = subDataset;
    m_datasetSize = m_subDataset.size();
    if (m_datasetSize == 0)
    {
        // cout << "Dataset of this sub model is empty" << endl;
        isValidNode = false;
        return;
    }
    isValidNode = true;
    m_secondStageNetwork.insert(m_subDataset, m_secondStageParams.maxEpoch, m_secondStageParams.learningRate, m_secondStageParams.neuronNumber);
    m_secondStageNetwork.train();
    int maxError = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        int p = predict(m_subDataset[i].first);
        int y = int(m_subDataset[i].second);
        int error = y - p;
        if (error > maxPositiveError)
            maxPositiveError = error;
        if (error < maxNegativeError)
            maxNegativeError = error;
        if (abs(error) > maxError)
            maxError = abs(error);
    }
    if (maxError > m_threshold)
    {
        isUseTree = true;
        for (int i = 0; i < m_datasetSize; i++)
        {
            m_tree.insert({m_subDataset[i].first, int(m_subDataset[i].second)});
        }
    }
    else
    {
        isUseTree = false;
    }
}

class totalModel
{
public:
    totalModel(const vector<pair<double, double>> &dataset, params firstStageParams, params secondStageParams, int secondStageThreshold, int secondStageSize, int maxInsertNumber)
    {
        m_dataset = dataset;
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
        m_secondStageSize = secondStageSize;
        m_secondStageThreshold = secondStageThreshold;
        m_maxInsertNumber = maxInsertNumber;

        m_firstStageNetwork.insert(m_dataset, m_firstStageParams.maxEpoch, m_firstStageParams.learningRate, m_firstStageParams.neuronNumber);

        for (int i = 0; i < m_secondStageSize; i++)
        {
            m_secondStage.push_back(secondStageNode(secondStageThreshold, m_secondStageParams));
        }
    }

    void sortData()
    {
        std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
    }

    void train();

    pair<double, double> find(double key);

    void insert(double key, double value)
    {
        m_insertArray.push_back({key, value});
        if (m_maxInsertNumber <= m_insertArray.size())
        {
            m_dataset.insert(m_dataset.end(), m_insertArray.begin(), m_insertArray.end());
            sortData();
            m_insertArray.clear();
            train();
        }
    }

private:
    vector<pair<double, double>> m_dataset;
    net m_firstStageNetwork = net();
    vector<secondStageNode> m_secondStage;
    params m_firstStageParams;
    params m_secondStageParams;
    int m_secondStageThreshold;
    int m_secondStageSize;

    vector<pair<double, double>> m_insertArray;
    int m_maxInsertNumber;
};

void totalModel::train()
{
    cout << "train first stage" << endl;
    m_firstStageNetwork.train();
    array<vector<pair<double, double>>, 128> perSubDataset;
    for (int i = 0; i < m_dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(m_dataset[i].first);
        p = p * (m_secondStageSize - 1); //calculate the index of second stage model
        int preIdx = static_cast<int>(p);
        // preIdx = max(0, preIdx);
        // preIdx = min(m_secondStageSize - 1, preIdx);
        perSubDataset[preIdx].push_back({m_dataset[i].first, double(i)});
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < m_secondStageSize; i++)
    {
        m_secondStage[i].train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

pair<double, double> totalModel::find(double key)
{
    // find key in m_insertArray firstly
    int res = -1;
    int insertStart = 0;
    int insertEnd = m_insertArray.size();
    while (insertStart < insertEnd)
    {
        int mid = (insertStart + insertEnd) / 2;
        if (m_insertArray[mid].first == key)
        {
            res = mid;
            break;
        }
        else if (m_insertArray[mid].first > key)
            insertEnd = mid;
        else
            insertStart = mid + 1;
    }
    if (res != -1)
    {
        return {key, m_insertArray[res].second};
    }


    double p = m_firstStageNetwork.predict(key);
    // cout << endl;
    // cout << "key: " << key << " , nn predict is: " << p;
    int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
    // preIdx = max(0, preIdx);
    // preIdx = min(m_secondStageSize - 1, preIdx);
    // cout << "    predict sub model is:" << preIdx << endl;
    if (m_secondStage[preIdx].validNode())
    {
        if (m_secondStage[preIdx].useTree())
        {
            auto res = m_secondStage[preIdx].treeFind(key);
            return {key, m_dataset[int(res.second)].second};
        }
        else
        {
            int preIndex = m_secondStage[preIdx].predict(key);
            if (m_dataset[preIndex].first == key)
            {
                return {key, m_dataset[preIndex].second};
            }
            else
            {
                int start = max(0, preIndex + m_secondStage[preIdx].getNagetive());
                int end = min(int(m_dataset.size() - 1), preIndex + m_secondStage[preIdx].getPositive());

                int res = -1;
                while (start < end)
                {
                    int mid = (start + end) / 2;
                    if (m_dataset[mid].first == key)
                    {
                        res = mid;
                        break;
                    }
                    else if (m_dataset[mid].first > key)
                        end = mid;
                    else
                        start = mid + 1;
                }

                if (res != -1)
                    return {key, m_dataset[res].second};
                else
                    return {};
            }
        }
    }
    else
    {
        cout << key << "   requested an invalid sub model" << endl;
        return {};
    }
}

#endif