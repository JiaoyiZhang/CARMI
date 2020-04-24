#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "inner_node.h"

template <typename lowerType>
class BinarySearchNode : public BasicInnerNode
{
public:
    BinarySearchNode() : BasicInnerNode(){};
    BinarySearchNode(int childNum) : BasicInnerNode(childNum){};
    BinarySearchNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(new lowerType(threshold, maxInsertNumber));
        }
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key);
    bool Insert(pair<double, double> data);
    bool Delete(double key);
    bool Update(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    vector<double> index;
};

template <typename lowerType>
void BinarySearchNode<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
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
        children[i]->Train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> BinarySearchNode<lowerType>::Find(double key)
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
    if (children[mid]->IsLeaf() == false)
        return ((BinarySearchNode *)children[mid])->Find(key);
    return children[mid]->Find(key);
}
template <typename lowerType>
bool BinarySearchNode<lowerType>::Update(pair<double, double> data)
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
    if (children[mid]->IsLeaf() == false)
        return ((BinarySearchNode *)children[mid])->Update(data);
    return children[mid]->Update(data);
}

template <typename lowerType>
bool BinarySearchNode<lowerType>::Delete(double key)
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
    if (children[mid]->IsLeaf() == false)
        return ((BinarySearchNode *)children[mid])->Delete(key);
    return children[mid]->Delete(key);
}

template <typename lowerType>
bool BinarySearchNode<lowerType>::Insert(pair<double, double> data)
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
    return children[mid]->Insert(data);
}

template <typename lowerType>
long double BinarySearchNode<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
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
        totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class AdaptiveBin : public BinarySearchNode<lowerType>
{
public:
    AdaptiveBin() : BinarySearchNode<lowerType>(){};
    AdaptiveBin(int maxKey, int childNum, int cap) : BinarySearchNode<lowerType>(childNum)
    {
        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    bool Insert(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data
};

template <typename lowerType>
void AdaptiveBin<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
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
            AdaptiveBin *child = new AdaptiveBin(maxKeyNum, this->childNumber, capacity);
            child->Initialize(perSubDataset[i]);
            this->children.push_back((lowerType *)child);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            lowerType *child = new lowerType(maxKeyNum, capacity);
            child->Train(perSubDataset[i]);
            this->children.push_back(child);
        }
    }
    cout << "End train" << endl;
}

template <typename lowerType>
bool AdaptiveBin<lowerType>::Insert(pair<double, double> data)
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
    int size = this->children[mid]->GetSize();

    // if an Insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[mid]->IsLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level moDelete in RMI
        // now becomes an inner level moDelete
        AdaptiveBin *newNode = new AdaptiveBin(maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[mid]->GetDataset(dataset);

        // a number of children leaf level moDeletes are created
        for (int i = 0; i < this->childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, capacity);
            newNode->children.push_back(temp);
        }

        // The data from the original leaf node is then
        // distributed to the newly created children leaf nodes
        // according to the original nodeÃ¢â‚¬â„¢s moDelete.
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
        // moDelete on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->Train(perSubDataset[i]);
        this->children[mid] = (lowerType *)newNode;
        return ((AdaptiveBin *)this->children[mid])->Insert(data);
    }
    else if (this->children[mid]->IsLeaf() == false)
        return ((AdaptiveBin *)this->children[mid])->Insert(data);
    return this->children[mid]->Insert(data);
}

template <typename lowerType>
long double AdaptiveBin<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
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
            totalCost += AdaptiveBin<lowerType>::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif