#ifndef DIVISION_NODE_H
#define DIVISION_NODE_H

#include "inner_node.h"
const int LOWER_ID2 = 0;
const int UPPER_ID2 = 2;
#define LOWER_TYPE2 ArrayNode
#define UPPER_TYPE2 AdaptiveDiv

class DivisionNode : public BasicInnerNode
{
public:
    DivisionNode() : BasicInnerNode(){};
    DivisionNode(int childNum) : BasicInnerNode(childNum){};
    DivisionNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(LeafNodeCreator(LOWER_ID2, threshold, maxInsertNumber));
            children_is_leaf.push_back(true);
        }
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

void DivisionNode::Initialize(const vector<pair<double, double>> &dataset)
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
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    cout << "End train" << endl;
}

pair<double, double> DivisionNode::Find(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children_is_leaf[preIdx] == false)
        return ((BasicInnerNode *)children[preIdx])->Find(key);
    return ((BasicLeafNode *)children[preIdx])->Find(key);
}

bool DivisionNode::Update(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children_is_leaf[preIdx] == false)
        return ((BasicInnerNode *)children[preIdx])->Update(data);
    return ((BasicLeafNode *)children[preIdx])->Update(data);
}

bool DivisionNode::Delete(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children_is_leaf[preIdx] == false)
        return ((BasicInnerNode *)children[preIdx])->Delete(key);
    return ((BasicLeafNode *)children[preIdx])->Delete(key);
}

bool DivisionNode::Insert(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    return ((BasicLeafNode *)children[preIdx])->Insert(data);
}

long double DivisionNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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
        totalCost += LOWER_TYPE2::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

class AdaptiveDiv : public DivisionNode
{
public:
    AdaptiveDiv() : DivisionNode(){};
    AdaptiveDiv(int maxKey, int childNum, int cap) : DivisionNode(childNum)
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

void AdaptiveDiv::Initialize(const vector<pair<double, double>> &dataset)
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
            UPPER_TYPE2 *child = (UPPER_TYPE2 *)InnerNodeCreator(UPPER_ID2, maxKeyNum, this->childNumber, capacity);
            child->Initialize(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(false);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            LOWER_TYPE2 *child = (LOWER_TYPE2 *)LeafNodeCreator(LOWER_ID2, maxKeyNum, capacity);
            child->SetDataset(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(true);
        }
    }
    cout << "End train" << endl;
}

bool AdaptiveDiv::Insert(pair<double, double> data)
{
    int preIdx = data.first / this->value;
    if (preIdx >= this->childNumber)
        preIdx = this->childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;

    if (this->children_is_leaf[preIdx] == true)
    {
        int size = ((BasicLeafNode *)this->children[preIdx])->GetSize();

        // if an Insert will push a leaf node's
        // data structure over its maximum bound number of keys,
        // then we split the leaf data node
        if (size >= maxKeyNum)
        {
            // The corresponding leaf level moDelete in RMI
            // now becomes an inner level moDelete
            UPPER_TYPE2 *newNode = (UPPER_TYPE2 *)InnerNodeCreator(UPPER_ID2, maxKeyNum, this->childNumber, capacity);
            vector<pair<double, double>> dataset;
            ((BasicLeafNode *)this->children[preIdx])->GetDataset(&dataset);

            double maxValue = dataset[dataset.size() - 1].first;
            double minValue = dataset[0].first;
            this->value = float(maxValue - minValue) / float(this->childNumber);

            // a number of children leaf level moDeletes are created
            for (int i = 0; i < this->childNumber; i++)
            {
                LOWER_TYPE2 *temp = (LOWER_TYPE2 *)LeafNodeCreator(LOWER_ID2, maxKeyNum, capacity);
                newNode->children.push_back(temp);
                newNode->children_is_leaf.push_back(true);
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
                ((BasicLeafNode *)(newNode->children[i]))->SetDataset(perSubDataset[i]);
            this->children[preIdx] = newNode;
            this->children_is_leaf[preIdx] = false;
            return ((BasicInnerNode *)this->children[preIdx])->Insert(data);
        }
    }
    else
        return ((BasicInnerNode *)this->children[preIdx])->Insert(data);
    return ((BasicLeafNode *)this->children[preIdx])->Insert(data);
}

long double AdaptiveDiv::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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
            totalCost += UPPER_TYPE2::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        }
        else
            totalCost += LOWER_TYPE2::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif