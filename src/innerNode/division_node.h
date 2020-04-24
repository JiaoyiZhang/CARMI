#ifndef DIVISION_NODE_H
#define DIVISION_NODE_H

#include "inner_node.h"

template <typename lowerType>
class DivisionNode : public BasicInnerNode
{
public:
    DivisionNode() : BasicInnerNode(){};
    DivisionNode(params secondStageParams, int childNum) : BasicInnerNode(childNum)
    {
        m_secondStageParams = secondStageParams;
    }
    DivisionNode(params secondStageParams, int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        m_secondStageParams = secondStageParams;

        for (int i = 0; i < childNumber; i++)
            children.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    float value;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void DivisionNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    value = float(maxValue - minValue) / float(childNumber);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / value;
        preIdx = min(preIdx, childNumber - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < childNumber; i++)
        children[i]->train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> DivisionNode<lowerType>::find(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((DivisionNode *)children[preIdx])->find(key);
    return children[preIdx]->find(key);
}
template <typename lowerType>
bool DivisionNode<lowerType>::update(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((DivisionNode *)children[preIdx])->update(data);
    return children[preIdx]->update(data);
}

template <typename lowerType>
bool DivisionNode<lowerType>::del(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((DivisionNode *)children[preIdx])->del(key);
    return children[preIdx]->del(key);
}

template <typename lowerType>
bool DivisionNode<lowerType>::insert(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    return children[preIdx]->insert(data);
}

template <typename lowerType>
long double DivisionNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    if (dataset.size() == 0)
        return 0;

    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    float value = float(maxValue - minValue) / float(childNum);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / value;
        preIdx = min(preIdx, childNum - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < childNum; i++)
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class AdaptiveDiv : public DivisionNode<lowerType>
{
public:
    AdaptiveDiv() : DivisionNode<lowerType>(){};
    AdaptiveDiv(params secondStageParams, int maxKey, int childNum, int cap) : DivisionNode<lowerType>(secondStageParams, childNum)
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
void AdaptiveDiv<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    this->value = float(maxValue - minValue) / float(this->childNumber);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < this->childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / this->value;
        preIdx = min(preIdx, this->childNumber - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < this->childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            AdaptiveDiv *child = new AdaptiveDiv(this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
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
bool AdaptiveDiv<lowerType>::insert(pair<double, double> data)
{
    int preIdx = data.first / this->value;
    if (preIdx >= this->childNumber)
        preIdx = this->childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;

    int size = this->children[preIdx]->getSize();

    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[preIdx]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        AdaptiveDiv *newNode = new AdaptiveDiv(this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[preIdx]->getDataset(dataset);

        std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
        double maxValue = dataset[dataset.size() - 1].first;
        double minValue = dataset[0].first;
        this->value = float(maxValue - minValue) / float(this->childNumber);

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
            int pIdx = data.first / this->value;
            perSubDataset[pIdx].push_back(dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->train(perSubDataset[i]);
        this->children[preIdx] = (lowerType *)newNode;
        return ((AdaptiveDiv *)this->children[preIdx])->insert(data);
    }
    else if (this->children[preIdx]->isLeaf() == false)
        return ((AdaptiveDiv *)this->children[preIdx])->insert(data);
    return this->children[preIdx]->insert(data);
}

template <typename lowerType>
long double AdaptiveDiv<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    if (dataset.size() == 0)
        return 0;

    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    float value = float(maxValue - minValue) / float(childNum);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / value;
        preIdx = min(preIdx, childNum - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxNum)
        {
            totalCost += AdaptiveBin<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        }
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif