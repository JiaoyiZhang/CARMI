#ifndef ADAPTIVE_RMI_H
#define ADAPTIVE_RMI_H

#include "../trainModel/lr.h"
#include "../trainModel/nn.h"
#include "../../cpp-btree/btree_map.h"
#include "../params.h"
#include <array>

template <typename lowerType, typename mlType>
class adaptiveRMI
{
public:
    adaptiveRMI(){};
    adaptiveRMI(params firstStageParams, params secondStageParams, int maxKey, int splitChildNumber, int cap)
    {
        // std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        //     return p1.first < p2.first;
        // });
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        childNumber = splitChildNumber;
        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void initialize(vector<pair<double, double>> &dataset);
    bool isLeaf() { return false; }
    void insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt); // insert data into new vector

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

private:
    vector<void *> children; // The type of child node may be innerNode or leafNode
    int childNumber;         // the number of children

    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data

    mlType m_firstStageNetwork = mlType(); // network of the first stage
    params m_firstStageParams;             // parameters of network
    params m_secondStageParams;            // parameters of lower nodes
};

// Initialize is first called on the RMI’s root
// node, and is then called recursively for all child nodes.
template <typename lowerType, typename mlType>
void adaptiveRMI<lowerType, mlType>::initialize(vector<pair<double, double>> &dataset)
{
    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    if (dataset.size() == 0)
        return;
    ;
    // first train the node’s linear model using its assigned keys
    m_firstStageNetwork.train(dataset, m_firstStageParams);

    //  use the model to divide the keys into some number of partitions
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < m_dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(m_dataset[i].first);
        p = p * (childNumber - 1); //calculate the index of second stage model
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(m_dataset[i]);
    }
    // then iterate through the partitions in sorted order
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            adaptiveRMI *child = new adaptiveRMI(m_firstStageParams, m_secondStageParams, maxKeyNum, childNumber, capacity);
            child->initialize(perSubDataset[i]);
            children.push_back(child);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            lowerType *child = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            child->train(perSubDataset[i]);
            children.push_back(child);
        }
    }
}

template <typename lowerType, typename mlType>
pair<double, double> adaptiveRMI<lowerType, mlType>::find(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    return children[preIdx]->find(key);
}

template <typename lowerType, typename mlType>
bool adaptiveRMI<lowerType, mlType>::insert(pair<double, double> data)
{
    double p = m_firstStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    int size = children[preIdx]->getSize();

    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (children[preIdx]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        adaptiveRMI *newNode = new adaptiveRMI(m_firstStageParams, m_secondStageParams, maxKeyNum, childNumber, capacity);
        vector<pair<double, double>> dataset = children[preIdx]->getDataset();

        // a number of children leaf level models are created
        for (int i = 0; i < childNumber; i++)
        {
            lowerType *tmp = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            newNode->children.push_back(tmp);
        }

        // The data from the original leaf node is then
        // distributed to the newly created children leaf nodes
        // according to the original node’s model.
        vector<vector<pair<double, double>>> perSubDataset;
        vector<pair<double, double>> temp;
        for (int i = 0; i < childNumber; i++)
        {
            perSubDataset.push_back(temp);
        }
        for (int i = 0; i < m_dataset.size(); i++)
        {
            double p = m_firstStageNetwork.predict(m_dataset[i].first);
            p = p * (childNumber - 1); 
            int pIdx = static_cast<int>(p);
            perSubDataset[pIdx].push_back(m_dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data. 
        for (int i = 0; i < childNumber; i++)
        {
            newNode->children[i]->train(perSubDataset[i]);
        }
        children[preIdx] = newNode;
    }
    return children[preIdx]->insert(data);
}

template <typename lowerType, typename mlType>
bool adaptiveRMI<lowerType, mlType>::del(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    return children[preIdx].del(key);
}

template <typename lowerType, typename mlType>
bool adaptiveRMI<lowerType, mlType>::update(pair<double, double> data)
{
    double p = m_firstStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    return children[preIdx]->update(data);
}

template <typename lowerType, typename mlType>
void adaptiveRMI<lowerType, mlType>::insertData(vector<pair<double, double>> &vec, pair<double, double> data, int idx, int &cnt)
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