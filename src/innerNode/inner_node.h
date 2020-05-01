#ifndef INNER_NODE_H
#define INNER_NODE_H

#include "../params.h"
#include "../leafNode/leaf_node.h"
#include "../trainModel/model.h"
#include "../../cpp-btree/btree_map.h"
#include <chrono>
#include <random>
#include <vector>
using namespace std;

extern int kLeafNodeID;
extern int kInnerNodeID;

extern const int kThreshold;
extern const double kDensity;
extern const int kMaxKeyNum;

extern BasicLeafNode *LeafNodeCreator(int leafNodeType);

class BasicInnerNode
{
public:
    BasicInnerNode(){};
    BasicInnerNode(int childNum)
    {
        childNumber = childNum;
    }

    void Initialize(const vector<pair<double, double>> &dataset);

    pair<double, double> Find(double key)
    {
        double p = model->Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Find(key);
        return ((BasicLeafNode *)children[preIdx])->Find(key);
    }
    bool Insert(pair<double, double> data)
    {
        double p = model->Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return ((BasicLeafNode *)children[preIdx])->Insert(data);
    }
    bool Delete(double key)
    {
        double p = model->Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Delete(key);
        return ((BasicLeafNode *)children[preIdx])->Delete(key);
    }
    bool Update(pair<double, double> data)
    {
        double p = model->Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Update(data);
        return ((BasicLeafNode *)children[preIdx])->Update(data);
    }

protected:
    vector<void *> children;       // store the lower nodes
    vector<bool> children_is_leaf; //record whether each child node is a leaf node
    int childNumber;               // the size of the lower nodes

    BasicModel *model; // model of the first stage
};

void BasicInnerNode::Initialize(const vector<pair<double, double>> &dataset)
{
    for (int i = 0; i < childNumber; i++)
    {
        children.push_back(LeafNodeCreator(kLeafNodeID));
        children_is_leaf.push_back(true);
    }

    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    model->Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = model->Predict(dataset[i].first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    cout << "End train" << endl;
}


#endif