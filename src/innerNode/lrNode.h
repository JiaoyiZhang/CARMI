#ifndef LR_NODE_H
#define LR_NODE_H

#include "innerNode.h"
#include "../trainModel/lr.h"

template <typename lowerType>
class lrNode : public basicInnerNode
{
public:
    lrNode() : basicInnerNode(){};
    lrNode(params firstStageParams, params secondStageParams, int childNum) : basicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
    }
    lrNode(params firstStageParams, params secondStageParams, int threshold, int childNum, int maxInsertNumber) : basicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        for (int i = 0; i < childNumber; i++)
            children.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->isLeaf() == false)
            return ((lrNode *)children[preIdx])->find(key);
        return children[preIdx]->find(key);
    }
    bool insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->insert(data);
    }
    bool del(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->isLeaf() == false)
            return ((lrNode *)children[preIdx])->del(key);
        return children[preIdx]->del(key);
    }
    bool update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children[preIdx]->isLeaf() == false)
            return ((lrNode *)children[preIdx])->update(data);
        return children[preIdx]->update(data);
    }

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    params m_firstStageParams;                                 // parameters of lr
    params m_secondStageParams;                                // parameters of lower nodes
    linearRegression m_firstStageNetwork = linearRegression(); // lr of the first stage
};

template <typename lowerType>
void lrNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    m_firstStageNetwork.train(dataset, m_firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return init(dataset);

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
        children[i]->train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
long double lrNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = 2;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    if (dataset.size() == 0)
        return 0;

    linearRegression tmpNet = linearRegression();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class adaptiveLR : public lrNode<lowerType>
{
public:
    adaptiveLR() : lrNode<lowerType>(){};
    adaptiveLR(params firstStageParams, params secondStageParams, int maxKey, int childNum, int cap) : lrNode<lowerType>(firstStageParams, secondStageParams, childNum)
    {
        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void init(const vector<pair<double, double>> &dataset);

    bool insert(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data
};

template <typename lowerType>
void adaptiveLR<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    // first train the node's linear model using its assigned keys
    this->m_firstStageNetwork.train(dataset, this->m_firstStageParams);
    //  use the model to divide the keys into some number of partitions
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < this->childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = this->m_firstStageNetwork.predict(dataset[i].first);
        p = p * (this->childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < this->childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return init(dataset);

    // then iterate through the partitions in sorted order
    cout << "train second stage" << endl;
    for (int i = 0; i < this->childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            adaptiveLR *child = new adaptiveLR(this->m_firstStageParams, this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
            child->init(perSubDataset[i]);
            this->children.push_back((lowerType *)child);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            lowerType *child = new lowerType(maxKeyNum, this->m_secondStageParams, capacity);
            child->train(perSubDataset[i]);
            this->children.push_back(child);
        }
    }
    cout << "End train" << endl;
}

template <typename lowerType>
bool adaptiveLR<lowerType>::insert(pair<double, double> data)
{
    double p = this->m_firstStageNetwork.predict(data.first);
    int preIdx = static_cast<int>(p * (this->childNumber - 1));
    int size = this->children[preIdx]->getSize();

    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[preIdx]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        adaptiveLR *newNode = new adaptiveLR(this->m_firstStageParams,this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[preIdx]->getDataset(dataset);
        newNode->m_firstStageNetwork.train(dataset, this->m_firstStageParams);

        // a number of children leaf level models are created
        for (int i = 0; i < this->childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, this->m_secondStageParams, capacity);
            newNode->children.push_back(temp);
        }

        // The data from the original leaf node is then
        // distributed to the newly created children leaf nodes
        // according to the original nodeÃ¢â‚¬â„¢s model.
        vector<vector<pair<double, double>>> perSubDataset;
        vector<pair<double, double>> temp;
        for (int i = 0; i < this->childNumber; i++)
            perSubDataset.push_back(temp);
        for (int i = 0; i < dataset.size(); i++)
        {
            double pre = newNode->m_firstStageNetwork.predict(dataset[i].first);
            pre = pre * (this->childNumber - 1);
            int pIdx = static_cast<int>(pre);
            perSubDataset[pIdx].push_back(dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->train(perSubDataset[i]);
        this->children[preIdx] = (lowerType *)newNode;
        return ((adaptiveLR *)this->children[preIdx])->insert(data);
    }
    else if (this->children[preIdx]->isLeaf() == false)
        return ((adaptiveLR *)this->children[preIdx])->insert(data);
    return this->children[preIdx]->insert(data);
}

template <typename lowerType>
long double adaptiveLR<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    if (dataset.size() == 0)
        return 0;

    linearRegression tmpNet = linearRegression();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxNum)
            totalCost += adaptiveNN<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif