#ifndef ADAPTIVE_RMI_H
#define ADAPTIVE_RMI_H

#include "../params.h"
#include <array>
#include "../leafNode/node.h"
#include "../leafNode/gappedNode.h"
using namespace std;
template <typename lowerType, typename mlType>
class adaptiveRMI
{
public:
    adaptiveRMI(){};
    adaptiveRMI(params firstStageParams, params secondStageParams, int maxKey, int splitChildNumber, int cap)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        childNumber = splitChildNumber;
        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
        isLeafNode = false;
    }

    void initialize(vector<pair<double, double>> &dataset);
    bool isLeaf() { return isLeafNode; }

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    void change(const vector<pair<int, int>> &cnt, int threshold, params secondStageParams, int cap);

private:
    vector<node *> children; // The type of child node may be innerNode or leafNode
    int childNumber;         // the number of children

    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data

    mlType m_firstStageNetwork = mlType(); // network of the first stage
    params m_firstStageParams;             // parameters of network
    params m_secondStageParams;            // parameters of lower nodes

    bool isLeafNode;
};

// Initialize is first called on the RMI’s root
// node, and is then called recursively for all child nodes.
template <typename lowerType, typename mlType>
void adaptiveRMI<lowerType, mlType>::initialize(vector<pair<double, double>> &dataset)
{
    cout << "Start initialize!" << endl;
    cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return;
    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });

    // first train the node's linear model using its assigned keys
    m_firstStageNetwork.train(dataset, m_firstStageParams);
    //  use the model to divide the keys into some number of partitions
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(dataset[i].first);
        p = p * (childNumber - 1); //calculate the index of second stage model
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() == dataset.size())
        {
            m_firstStageParams.learningRate2 *= 10;
            return initialize(dataset);
        }
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
            isLeafNode = false;
            adaptiveRMI *child = new adaptiveRMI(m_firstStageParams, m_secondStageParams, maxKeyNum, childNumber, capacity);
            child->initialize(perSubDataset[i]);
            children.push_back((lowerType *)child);
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
    return;
}

template <typename lowerType, typename mlType>
pair<double, double> adaptiveRMI<lowerType, mlType>::find(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveRMI *)children[preIdx])->find(key);
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
        vector<pair<double, double>> dataset;
        children[preIdx]->getDataset(dataset);
        newNode->m_firstStageNetwork.train(dataset, m_firstStageParams);

        // a number of children leaf level models are created
        for (int i = 0; i < childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            newNode->children.push_back(temp);
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
        for (int i = 0; i < dataset.size(); i++)
        {
            double pre = newNode->m_firstStageNetwork.predict(dataset[i].first);
            pre = pre * (childNumber - 1);
            int pIdx = static_cast<int>(pre);
            perSubDataset[pIdx].push_back(dataset[i]);
        }
        for (int i = 0; i < childNumber; i++)
        {
            if (perSubDataset[i].size() == size)
            {
                m_firstStageParams.learningRate1 *= 10;
                m_firstStageParams.learningRate2 *= 10;
                return insert(data);
            }
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < childNumber; i++)
        {
            newNode->children[i]->train(perSubDataset[i]);
        }
        children[preIdx] = (lowerType *)newNode;
        return ((adaptiveRMI *)children[preIdx])->insert(data);
    }
    else if (children[preIdx]->isLeaf() == false)
        return ((adaptiveRMI *)children[preIdx])->insert(data);
    return children[preIdx]->insert(data);
}

template <typename lowerType, typename mlType>
bool adaptiveRMI<lowerType, mlType>::del(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveRMI *)children[preIdx])->del(key);
    return children[preIdx]->del(key);
}

template <typename lowerType, typename mlType>
bool adaptiveRMI<lowerType, mlType>::update(pair<double, double> data)
{
    double p = m_firstStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveRMI *)children[preIdx])->update(data);
    return children[preIdx]->update(data);
}

template <typename lowerType, typename mlType>
void adaptiveRMI<lowerType, mlType>::change(const vector<pair<int, int>> &cnt, int threshold, params secondStageParams, int cap)
{
    int idx = 0;
    for (int i = 0; i < childNumber; i++)
    {
        if (children[i]->isLeaf())
        {
            int r = 0, w = 0;
            vector<pair<double, double>> tmp;
            children[i]->getDataset(tmp);
            vector<pair<double, double>> data;
            for (int j = 0; j < tmp.size(); j++)
            {
                if (tmp[j].first != -1)
                {
                    r += cnt[idx].first;
                    w += cnt[idx].second;
                    data.push_back(tmp[j]);
                    idx++;
                }
            }
            if ((float)r / (float)w <= 5.0)
            {
                // change from array to gapped array
                gappedNode<mlType> *newNode = new gappedNode<mlType>(threshold, secondStageParams, cap);
                newNode->train(data);
                children[i] = newNode;
            }
        }
    }
}

#endif