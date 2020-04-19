#ifndef ADAPTIVE_LR_H
#define ADAPTIVE_LR_H

#include "innerNode.h"
#include "../trainModel/lr.h"

template <typename lowerType>
class adaptiveLR : public basicInnerNode
{
public:
    adaptiveLR() : basicInnerNode(){};
    adaptiveLR(params firstStageParams, params secondStageParams, int maxKey, int childNum, int cap) : basicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data

    params m_firstStageParams;                                 // parameters of lr
    params m_secondStageParams;                                // parameters of lower nodes
    linearRegression m_firstStageNetwork = linearRegression(); // lr of the first stage
};

template <typename lowerType>
void adaptiveLR<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });

    cout << "train first stage" << endl;
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
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() == dataset.size())
            return init(dataset);
    }

    // then iterate through the partitions in sorted order
    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            adaptiveLR *child = new adaptiveLR(m_firstStageParams, m_secondStageParams, maxKeyNum, childNumber, capacity);
            child->init(perSubDataset[i]);
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
    cout << "End train" << endl;
}


template <typename lowerType>
pair<double, double> adaptiveLR<lowerType>::find(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveLR *)children[preIdx])->find(key);
    return children[preIdx]->find(key);
}

template <typename lowerType>
bool adaptiveLR<lowerType>::insert(pair<double, double> data)
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
        adaptiveLR *newNode = new adaptiveLR(m_firstStageParams, m_secondStageParams, maxKeyNum, childNumber, capacity);
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
        // according to the original nodeâ€™s model.
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
        return ((adaptiveLR *)children[preIdx])->insert(data);
    }
    else if (children[preIdx]->isLeaf() == false)
        return ((adaptiveLR *)children[preIdx])->insert(data);
    return children[preIdx]->insert(data);
}

template <typename lowerType>
bool adaptiveLR<lowerType>::del(double key)
{
    double p = m_firstStageNetwork.predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveLR *)children[preIdx])->del(key);
    return children[preIdx]->del(key);
}

template <typename lowerType>
bool adaptiveLR<lowerType>::update(pair<double, double> data)
{
    double p = m_firstStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveLR *)children[preIdx])->update(data);
    return children[preIdx]->update(data);
}

template <typename lowerType>
long double adaptiveLR<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    // cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return 0;

    linearRegression tmpNet = linearRegression();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
            totalCost += adaptiveNN<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif