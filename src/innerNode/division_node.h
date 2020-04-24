#ifndef DIVISION_NODE_H
#define DIVISION_NODE_H

#include "inner_node.h"

template <typename lowerType>
class DivisionNode : public BasicInnerNode
{
public:
    DivisionNode() : BasicInnerNode(){};
    DivisionNode(int childNum) : BasicInnerNode(childNum){};
    DivisionNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
            children.push_back(new lowerType(threshold, maxInsertNumber));
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key);
    bool Insert(pair<double, double> data);
    bool Delete(double key);
    bool Update(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    float value;
};

template <typename lowerType>
void DivisionNode<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
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
        children[i]->Train(perSubDataset[i]);
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> DivisionNode<lowerType>::Find(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->IsLeaf() == false)
        return ((DivisionNode *)children[preIdx])->Find(key);
    return children[preIdx]->Find(key);
}
template <typename lowerType>
bool DivisionNode<lowerType>::Update(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->IsLeaf() == false)
        return ((DivisionNode *)children[preIdx])->Update(data);
    return children[preIdx]->Update(data);
}

template <typename lowerType>
bool DivisionNode<lowerType>::Delete(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->IsLeaf() == false)
        return ((DivisionNode *)children[preIdx])->Delete(key);
    return children[preIdx]->Delete(key);
}

template <typename lowerType>
bool DivisionNode<lowerType>::Insert(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    return children[preIdx]->Insert(data);
}

template <typename lowerType>
long double DivisionNode<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
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
        totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

template <typename lowerType>
class AdaptiveDiv : public DivisionNode<lowerType>
{
public:
    AdaptiveDiv() : DivisionNode<lowerType>(){};
    AdaptiveDiv(int maxKey, int childNum, int cap) : DivisionNode<lowerType>(childNum)
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
void AdaptiveDiv<lowerType>::Initialize(const vector<pair<double, double>> &dataset)
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
            AdaptiveDiv *child = new AdaptiveDiv(maxKeyNum, this->childNumber, capacity);
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
bool AdaptiveDiv<lowerType>::Insert(pair<double, double> data)
{
    int preIdx = data.first / this->value;
    if (preIdx >= this->childNumber)
        preIdx = this->childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;

    int size = this->children[preIdx]->GetSize();

    // if an Insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (this->children[preIdx]->IsLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level moDelete in RMI
        // now becomes an inner level moDelete
        AdaptiveDiv *newNode = new AdaptiveDiv(maxKeyNum, this->childNumber, capacity);
        vector<pair<double, double>> dataset;
        this->children[preIdx]->GetDataset(dataset);

        std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
        double maxValue = dataset[dataset.size() - 1].first;
        double minValue = dataset[0].first;
        this->value = float(maxValue - minValue) / float(this->childNumber);

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
        vector<pair<double, double>> temp;
        for (int i = 0; i < this->childNumber; i++)
            perSubDataset.push_back(temp);
        for (int i = 0; i < dataset.size(); i++)
        {
            int pIdx = data.first / this->value;
            perSubDataset[pIdx].push_back(dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // moDelete on its portion of the data.
        for (int i = 0; i < this->childNumber; i++)
            newNode->children[i]->Train(perSubDataset[i]);
        this->children[preIdx] = (lowerType *)newNode;
        return ((AdaptiveDiv *)this->children[preIdx])->Insert(data);
    }
    else if (this->children[preIdx]->IsLeaf() == false)
        return ((AdaptiveDiv *)this->children[preIdx])->Insert(data);
    return this->children[preIdx]->Insert(data);
}

template <typename lowerType>
long double AdaptiveDiv<lowerType>::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
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
            totalCost += AdaptiveBin<lowerType>::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        }
        else
            totalCost += lowerType::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif