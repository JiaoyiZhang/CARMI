#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "innerNode.h"

template <typename lowerType>
class binarySearchNode : public basicInnerNode
{
public:
    binarySearchNode() : basicInnerNode(){};
    binarySearchNode(params secondStageParams, int childNum) : basicInnerNode(childNum)
    {
        m_secondStageParams = secondStageParams;
    }
    binarySearchNode(params secondStageParams, int threshold, int childNum, int maxInsertNumber) : basicInnerNode(childNum)
    {
        m_secondStageParams = secondStageParams;

        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
        }
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    vector<double> index;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void binarySearchNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < dataset.size(); i++)
    {
        tmp.push_back(dataset[i]);
        if ((i + 1) % (dataset.size() / childNumber) == 0)
        {
            perSubDataset.push_back(tmp);
            index.push_back(dataset[i].first);
            tmp = vector<pair<double, double>>();
        }
    }
    if (index.size() == childNumber - 1)
    {
        perSubDataset.push_back(tmp);
        index.push_back(dataset[dataset.size() - 1].first);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < childNumber; i++)
        children[i]->train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> binarySearchNode<lowerType>::find(double key)
{
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx <= end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if ((index[mid] >= key && (index[mid - 1] < key || mid == 0)) || mid == childNumber - 1)
            break;
        if (index[mid] < key)
            start_idx = mid + 1;
        else if (index[mid] > key)
            end_idx = mid;
        else
            break;
    }
    if (children[mid]->isLeaf() == false)
        return ((binarySearchNode *)children[mid])->find(key);
    return children[mid]->find(key);
}
template <typename lowerType>
bool binarySearchNode<lowerType>::update(pair<double, double> data)
{
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx <= end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if ((index[mid] >= data.first && (index[mid - 1] < data.first || mid == 0)) || mid == childNumber - 1)
            break;
        if (index[mid] < data.first)
            start_idx = mid + 1;
        else if (index[mid] > data.first)
            end_idx = mid;
        else
            break;
    }
    if (children[mid]->isLeaf() == false)
        return ((binarySearchNode *)children[mid])->update(data);
    return children[mid]->update(data);
}

template <typename lowerType>
bool binarySearchNode<lowerType>::del(double key)
{
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx <= end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if ((index[mid] >= key && (index[mid - 1] < key || mid == 0)) || mid == childNumber - 1)
            break;
        if (index[mid] < key)
            start_idx = mid + 1;
        else if (index[mid] > key)
            end_idx = mid;
        else
            break;
    }
    if (children[mid]->isLeaf() == false)
        return ((binarySearchNode *)children[mid])->del(key);
    return children[mid]->del(key);
}

template <typename lowerType>
bool binarySearchNode<lowerType>::insert(pair<double, double> data)
{
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx <= end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if ((index[mid] >= data.first && (index[mid - 1] < data.first || mid == 0)) || mid == childNumber - 1)
            break;
        if (index[mid] < data.first)
            start_idx = mid + 1;
        else if (index[mid] > data.first)
            end_idx = mid;
        else
            break;
    }
    return children[mid]->insert(data);
}

template <typename lowerType>
long double binarySearchNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    if (dataset.size() == 0)
        return 0;

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < dataset.size(); i++)
    {
        tmp.push_back(dataset[i]);
        if ((i + 1) % (dataset.size() / childNum) == 0)
        {
            perSubDataset.push_back(tmp);
            tmp = vector<pair<double, double>>();
        }
    }
    perSubDataset.push_back(tmp);

    for (int i = 0; i < childNum; i++)
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class adaptiveBin : public binarySearchNode<lowerType>
{
public:
    adaptiveBin() : binarySearchNode<lowerType>(){};
    adaptiveBin(params secondStageParams, int maxKey, int childNum, int cap) : binarySearchNode<lowerType>(secondStageParams, childNum)
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
void adaptiveBin<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < dataset.size(); i++)
    {
        tmp.push_back(dataset[i]);
        if ((i + 1) % (dataset.size() / this->childNumber) == 0)
        {
            perSubDataset.push_back(tmp);
            this->index.push_back(dataset[i].first);
            tmp = vector<pair<double, double>>();
        }
    }
    if (this->index.size() == this->childNumber - 1)
    {
        perSubDataset.push_back(tmp);
        this->index.push_back(dataset[dataset.size() - 1].first);
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
            adaptiveBin *child = new adaptiveBin(this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
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
bool adaptiveBin<lowerType>::insert(pair<double, double> data)
{
    int start_idx = 0;
    int end_idx = this->childNumber - 1;
    int mid;
    while (start_idx <= end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if ((this->index[mid] >= data.first && (this->index[mid - 1] < data.first || mid == 0)) || mid == this->childNumber - 1)
            break;
        if (this->index[mid] < data.first)
            start_idx = mid + 1;
        else if (this->index[mid] > data.first)
            end_idx = mid;
        else
            break;
    }
    int size = this->children[mid]->getSize();

    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[mid]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        adaptiveBin *newNode = new adaptiveBin(this->m_secondStageParams, maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[mid]->getDataset(dataset);

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
        vector<pair<double, double>> tmp;
        for (int i = 0; i < dataset.size(); i++)
        {
            tmp.push_back(dataset[i]);
            if ((i + 1) % (dataset.size() / newNode->childNumber) == 0)
            {
                perSubDataset.push_back(tmp);
                newNode->index.push_back(dataset[i].first);
                tmp = vector<pair<double, double>>();
            }
        }
        if (newNode->index.size() == this->childNumber - 1)
        {
            perSubDataset.push_back(tmp);
            newNode->index.push_back(dataset[dataset.size() - 1].first);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->train(perSubDataset[i]);
        this->children[mid] = (lowerType *)newNode;
        return ((adaptiveBin *)this->children[mid])->insert(data);
    }
    else if (this->children[mid]->isLeaf() == false)
        return ((adaptiveBin *)this->children[mid])->insert(data);
    return this->children[mid]->insert(data);
}

template <typename lowerType>
long double adaptiveBin<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    // cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return 0;

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < dataset.size(); i++)
    {
        tmp.push_back(dataset[i]);
        if ((i + 1) % (dataset.size() / childNum) == 0)
        {
            perSubDataset.push_back(tmp);
            tmp = vector<pair<double, double>>();
        }
    }
    perSubDataset.push_back(tmp);

    // then iterate through the partitions in sorted order
    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxNum)
            totalCost += adaptiveBin<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif