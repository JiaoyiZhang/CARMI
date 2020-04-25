#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "inner_node.h"

const int LOWER_ID3 = 0;
const int UPPER_ID3 = 3;
#define LOWER_TYPE3 ArrayNode
#define UPPER_TYPE3 AdaptiveBin

class BinarySearchNode : public BasicInnerNode
{
public:
    BinarySearchNode() : BasicInnerNode(){};
    BinarySearchNode(int childNum) : BasicInnerNode(childNum){};
    BinarySearchNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(LeafNodeCreator(LOWER_ID3, threshold, maxInsertNumber));
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
    vector<double> index;
};

void BinarySearchNode::Initialize(const vector<pair<double, double>> &dataset)
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
            tmp.clear();
        }
    }
    if (index.size() == childNumber - 1)
    {
        perSubDataset.push_back(tmp);
        index.push_back(dataset[dataset.size() - 1].first);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < childNumber; i++)
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    cout << "End train" << endl;
}

pair<double, double> BinarySearchNode::Find(double key)
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
    if (children_is_leaf[mid] == false)
        return ((BasicInnerNode *)children[mid])->Find(key);
    return ((BasicLeafNode *)children[mid])->Find(key);
}

bool BinarySearchNode::Update(pair<double, double> data)
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
    if (children_is_leaf[mid] == false)
        return ((BasicInnerNode *)children[mid])->Update(data);
    return ((BasicLeafNode *)children[mid])->Update(data);
}

bool BinarySearchNode::Delete(double key)
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
    if (children_is_leaf[mid] == false)
        return ((BasicInnerNode *)children[mid])->Delete(key);
    return ((BasicLeafNode *)children[mid])->Delete(key);
}

bool BinarySearchNode::Insert(pair<double, double> data)
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
    return ((BasicLeafNode *)children[mid])->Insert(data);
}

long double BinarySearchNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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
        totalCost += LOWER_TYPE3::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

class AdaptiveBin : public BinarySearchNode
{
public:
    AdaptiveBin() : BinarySearchNode(){};
    AdaptiveBin(int maxKey, int childNum, int cap) : BinarySearchNode(childNum)
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

void AdaptiveBin::Initialize(const vector<pair<double, double>> &dataset)
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
            UPPER_TYPE3 *child = (UPPER_TYPE3 *)InnerNodeCreator(UPPER_ID3, maxKeyNum, this->childNumber, capacity);
            child->Initialize(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(false);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            LOWER_TYPE3 *child = (LOWER_TYPE3 *)LeafNodeCreator(LOWER_ID3, maxKeyNum, capacity);
            child->SetDataset(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(true);
        }
    }
    cout << "End train" << endl;
}

bool AdaptiveBin::Insert(pair<double, double> data)
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
    if (this->children_is_leaf[mid] == true)
    {
        int size = ((BasicLeafNode *)this->children[mid])->GetSize();
        // if an Insert will push a leaf node's
        // data structure over its maximum bound number of keys,
        // then we split the leaf data node
        if (size >= maxKeyNum)
        {
            // The corresponding leaf level moDelete in RMI
            // now becomes an inner level moDelete
            UPPER_TYPE3 *newNode = (UPPER_TYPE3 *)InnerNodeCreator(UPPER_ID3, maxKeyNum, this->childNumber, capacity);
            vector<pair<double, double>> dataset;
            ((BasicLeafNode *)this->children[mid])->GetDataset(&dataset);

            // a number of children leaf level moDeletes are created
            for (int i = 0; i < this->childNumber; i++)
            {
                LOWER_TYPE3 *temp = (LOWER_TYPE3 *)LeafNodeCreator(LOWER_ID3, maxKeyNum, capacity);
                newNode->children_is_leaf.push_back(true);
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
                ((BasicLeafNode *)(newNode->children[i]))->SetDataset(perSubDataset[i]);
            this->children[mid] = newNode;
            return ((BasicInnerNode *)this->children[mid])->Insert(data);
        }
    }
    else
        return ((BasicInnerNode *)this->children[mid])->Insert(data);
    return ((BasicLeafNode *)this->children[mid])->Insert(data);
}

long double AdaptiveBin::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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
            totalCost += UPPER_TYPE3::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += LOWER_TYPE3::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif