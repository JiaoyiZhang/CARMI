#ifndef LR_NODE_H
#define LR_NODE_H

#include "inner_node.h"
#include "../trainModel/lr.h"

extern BasicInnerNode *InnerNodeCreator(int innerNodeType, int childNum);

class LRNode : public BasicInnerNode
{
public:
    LRNode() : BasicInnerNode(){};
    LRNode(int childNum) : BasicInnerNode(childNum)
    {
        model = new LinearRegression();
    }

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &dataset);
};

long double LRNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &dataset)
{
    // space consumption: 16
    // calculation: 2
    // here is 16/10 + 2
    double InitializeCost = 3.6;
    // cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpModel = LinearRegression();
    tmpModel.Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpModel.Predict(dataset[i].first);
        p = p * (childNum - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
        totalCost += LEAF_NODE_TYPE::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}
class AdaptiveLR : public LRNode
{
public:
    AdaptiveLR() : LRNode(){};
    AdaptiveLR(int childNum) : LRNode(childNum)
    {
        model = new LinearRegression();
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    bool Insert(pair<double, double> data);

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &dataset);
};

void AdaptiveLR::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    this->model->Train(dataset);
    for (int i = 0; i < childNumber; i++)
    {
        children.push_back(LeafNodeCreator(kLeafNodeID));
        children_is_leaf.push_back(true);
    }
    auto tmpDataset = dataset;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(tmpDataset.begin(), tmpDataset.end(), default_random_engine(seed));
    for (int i = 0; i < tmpDataset.size(); i++)
    {
        Insert(tmpDataset[i]);
    }
    cout << "End train" << endl;
}

bool AdaptiveLR::Insert(pair<double, double> data)
{
    double p = this->model->Predict(data.first);
    int preIdx = static_cast<int>(p * (this->childNumber - 1));
    if (this->children_is_leaf[preIdx] == true)
    {
        int size = ((BasicLeafNode *)this->children[preIdx])->GetSize();
        // if an Insert will push a leaf node's
        // data structure over its maximum bound number of keys,
        // then we split the leaf data node
        if (size >= kMaxKeyNum)
        {
            // The corresponding leaf level model in RMI
            // now becomes an inner level model
            AdaptiveLR *newNode = (AdaptiveLR *)InnerNodeCreator(kInnerNodeID, this->childNumber);
            vector<pair<double, double>> dataset;
            ((BasicLeafNode *)this->children[preIdx])->GetDataset(&dataset);
            newNode->model->Train(dataset);

            // a number of children leaf level model are created
            for (int i = 0; i < this->childNumber; i++)
            {
                LEAF_NODE_TYPE *temp = (LEAF_NODE_TYPE *)LeafNodeCreator(kLeafNodeID);
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
                double pre = newNode->model->Predict(dataset[i].first);
                int pIdx = static_cast<int>(pre * (childNumber - 1));
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

long double AdaptiveLR::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &dataset)
{
    // space consumption: 16
    // calculation: 2
    // here is 16/10 + 2
    double InitializeCost = 3.6;
    // cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    LinearRegression tmpModel = LinearRegression();
    tmpModel.Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNum; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpModel.Predict(dataset[i].first);
        int preIdx = static_cast<int>(p * (childNum - 1));
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > kMaxKeyNum)
            totalCost += AdaptiveLR::GetCost(cntTree, childNum, perSubDataset[i]);
        else
            totalCost += LEAF_NODE_TYPE::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif