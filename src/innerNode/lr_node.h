#ifndef LR_NODE_H
#define LR_NODE_H

#include "inner_node.h"
#include "../trainModel/lr.h"

#define LOWER_TYPE0 ArrayNode
// #define LOWER_TYPE0 GappedArray
#define UPPER_TYPE0 AdaptiveLR

const int LOWER_ID0 = 0;
const int UPPER_ID0 = 0;

class LRNode : public BasicInnerNode
{
public:
    LRNode() : BasicInnerNode(){};
    LRNode(int childNum) : BasicInnerNode(childNum){};
    LRNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(LeafNodeCreator(LOWER_ID0, threshold, maxInsertNumber));
            children_is_leaf.push_back(true);
        }
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key)
    {
        double p = m_firstStageNetwork.Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Find(key);
        return ((BasicLeafNode*)children[preIdx])->Find(key);
    }
    bool Insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return ((BasicLeafNode*)children[preIdx])->Insert(data);
    }
    bool Delete(double key)
    {
        double p = m_firstStageNetwork.Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Delete(key);
        return ((BasicLeafNode*)children[preIdx])->Delete(key);
    }
    bool Update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Update(data);
        return ((BasicLeafNode*)children[preIdx])->Update(data);
    }

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    LinearRegression m_firstStageNetwork = LinearRegression(); // lr of the first stage
};


void LRNode::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    m_firstStageNetwork.Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = m_firstStageNetwork.Predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return Initialize(dataset);

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    cout << "End train" << endl;
}


long double LRNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 2;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpNet = LinearRegression();
    tmpNet.Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.Predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
        totalCost += LOWER_TYPE0::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}


class AdaptiveLR : public LRNode
{
public:
    AdaptiveLR() : LRNode(){};
    AdaptiveLR(int maxKey, int childNum, int cap) : LRNode(childNum)
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


void AdaptiveLR::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    // first train the node's linear moDelete using its assigned keys
    this->m_firstStageNetwork.Train(dataset);
    //  use the moDelete to divide the keys into some number of partitions
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < this->childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = this->m_firstStageNetwork.Predict(dataset[i].first);
        p = p * (this->childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < this->childNumber; i++)
        if (perSubDataset[i].size() == dataset.size())
            return Initialize(dataset);

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
            UPPER_TYPE0 *child = (UPPER_TYPE0 *)InnerNodeCreator(UPPER_ID0, maxKeyNum, this->childNumber, capacity);
            child->Initialize(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(false);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            LOWER_TYPE0 *child = (LOWER_TYPE0 *)LeafNodeCreator(LOWER_ID0, maxKeyNum, capacity);
            child->SetDataset(perSubDataset[i]);
            this->children.push_back(child);
            this->children_is_leaf.push_back(true);
        }
    }
    cout << "End train" << endl;
}


bool AdaptiveLR::Insert(pair<double, double> data)
{
    double p = this->m_firstStageNetwork.Predict(data.first);
    int preIdx = static_cast<int>(p * (this->childNumber - 1));
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
            UPPER_TYPE0 *newNode = (UPPER_TYPE0 *)InnerNodeCreator(UPPER_ID0, maxKeyNum, this->childNumber, capacity);
            vector<pair<double, double>> dataset;
            ((BasicLeafNode *)this->children[preIdx])->GetDataset(&dataset);
            newNode->m_firstStageNetwork.Train(dataset);

            // a number of children leaf level moDeletes are created
            for (int i = 0; i < this->childNumber; i++)
            {
            LOWER_TYPE0 *temp = (LOWER_TYPE0 *)LeafNodeCreator(LOWER_ID0, maxKeyNum, capacity);
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
                double pre = newNode->m_firstStageNetwork.Predict(dataset[i].first);
                pre = pre * (this->childNumber - 1);
                int pIdx = static_cast<int>(pre);
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
    return ((BasicLeafNode*)this->children[preIdx])->Insert(data);
}


long double AdaptiveLR::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpNet = LinearRegression();
    tmpNet.Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.Predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxNum)
            totalCost += UPPER_TYPE0::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += LOWER_TYPE0::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif