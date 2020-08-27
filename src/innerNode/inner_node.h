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

extern int kThreshold;
extern const double kDensity;
extern const int kAdaptiveChildNum;
extern double kRate;
extern const double kReadWriteRate;
extern int kMaxKeyNum;

extern BasicLeafNode *LeafNodeCreator(int leafNodeType);

class BasicInnerNode
{
public:
    BasicInnerNode(){};
    BasicInnerNode(int childNum)
    {
        childNumber = childNum;
    }

    // designed for test
    int GetChildNum(){return children_is_leaf.size();}
    void GetChildSet(vector<pair<double, double>> *v, double key)
    {
        double p = model->Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        cout<<"PreIdx is:"<<preIdx<<endl;
        ((BasicLeafNode *)children[preIdx])->GetTotalDataset(v);
    }
    void GetLeafNode(double key);

    // designed for reconstruction
    void PrintStructure();
    void Rebuild(const vector<pair<double, double>> &total, const vector<pair<double, double>> &dataset, const btree::btree_map<double, pair<int, int>> &cntTree);

    void Initialize(const vector<pair<double, double>> &dataset);
    void Release();

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

    double CalculateError(int num)
    {
        double error=0;
        for(int i=0;i<childNumber;i++)
        {
            error += ((BasicLeafNode *)children[i])->CalculateError();            
        }
        cout<<"total error: "<<error<<endl;
        cout<<"average error: "<<error/float(num);
        return error;
    }

    int CalculateSpaceCost()
    {
        int cost = 5 * childNumber + 4;
        switch (kInnerNodeID)
        {
        case 0:  // lr
            cost += 16;
            break;
        case 1:  // nn
            cost += 200;
            break;
        case 2:
            cost += 16 * childNumber + 16;
            break;
        default:
            cost += 8 * childNumber;
            break;
        }
        for(int i=0;i<childNumber;i++)
        {
            if (children_is_leaf[i] == false)
                cost += ((BasicInnerNode *)children[i])->CalculateSpaceCost();
        }
        cout<<"total space cost is: "<<float(cost) / 1048576.0<<endl;
        return cost;
    }

    void UpdateError(const vector<pair<double, double>> &findDataset, const vector<pair<double, double>> &insertDataset)
    {
        vector<vector<pair<double, double>>> subFind, subInsert;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < childNumber; i++)
        {
            subFind.push_back(tmp);
            subInsert.push_back(tmp);
        }

        for (int i = 0; i < findDataset.size(); i++)
        {
            double p = model->Predict(findDataset[i].first);
            int preIdx = static_cast<int>(p * (childNumber - 1));
            subFind[preIdx].push_back(findDataset[i]);
        }
        for (int i = 0; i < insertDataset.size(); i++)
        {
            double p = model->Predict(insertDataset[i].first);
            int preIdx = static_cast<int>(p * (childNumber - 1));
            subInsert[preIdx].push_back(insertDataset[i]);
        }

        double error = 0, size = 0;
        for (int i = 0; i < childNumber; i++)
        {
            error += ((BasicLeafNode *)children[i])->UpdateError(subFind[i], subInsert[i]);
            size += ((BasicLeafNode *)children[i])->GetSize()*((BasicLeafNode *)children[i])->GetSize();
        }
        cout<<"Final error is: "<<error<<"\t size is:"<<size<<endl;
        cout << "average log error: " << error / findDataset.size() << endl;
        cout << "average size: " << size / findDataset.size() << endl;     
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
    double total=0;
    for (int i = 0; i < childNumber; i++)
    {
        total += perSubDataset[i].size()*perSubDataset[i].size();
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    }
    cout<<"ave size:"<<float(total)/float(dataset.size())<<endl;
    cout << "End train" << endl;
}


void BasicInnerNode::GetLeafNode(double key)
{
    double p = model->Predict(key);
    int preIdx = static_cast<int>(p * (childNumber - 1));
    if (children_is_leaf[preIdx] == false)
        return ((BasicInnerNode *)children[preIdx])->GetLeafNode(key);
    else
    {
        vector<pair<double, double>> subDataset;
        ((BasicLeafNode *)children[preIdx])->GetTotalDataset(&subDataset);
        int pIdx = ((GappedArray *)children[preIdx])->GetPredictIndex(key);
        int maxError = ((BasicLeafNode *)children[preIdx])->GetError();
        int maxIndex = ((GappedArray *)children[preIdx])->GetMaxIndex();
        int start = max(0, pIdx - maxError);
        int end = min(maxIndex, pIdx + maxError);
        int res = ((GappedArray *)children[preIdx])->BinarySearch(key, pIdx, start, end);
        if (res <= start)
            res = ((GappedArray *)children[preIdx])->BinarySearch(key, pIdx, 0, start);
        else if (res >= end)
            res = ((GappedArray *)children[preIdx])->BinarySearch(key, pIdx, res, maxIndex);
    }
}


void BasicInnerNode::PrintStructure()
{
    cout << "Root!" << endl;
    for(int i = 0; i < childNumber; i++)
    {
        if (children_is_leaf[i] == false)
            cout<<"inner"<<"\t";
        else
            cout<<"leaf"<<"\t";
    }
    cout << endl;
    for(int i = 0; i < childNumber; i++)
    {
        if (children_is_leaf[i] == false)
        {
            cout<<"child: "<<i<<endl;
            ((BasicInnerNode *)children[i])->PrintStructure();
        }
    }
}


void BasicInnerNode::Rebuild(const vector<pair<double, double>> &total, const vector<pair<double, double>> &dataset, const btree::btree_map<double, pair<int, int>> &cntTree)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    model->Train(dataset);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
        perSubDataset.push_back(tmp);

    for (int i = 0; i < total.size(); i++)
    {
        double p = model->Predict(total[i].first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        perSubDataset[preIdx].push_back(total[i]);
    }
    vector<vector<pair<double, double>>> perSubInit;
    for (int i = 0; i < childNumber; i++)
        perSubInit.push_back(tmp);

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = model->Predict(dataset[i].first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        perSubInit[preIdx].push_back(dataset[i]);
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
    {
        if(perSubDataset[i].size() == 0)
        {
            cout<<"child "<<i<<" is empty, use ga!"<<endl;
            children.push_back(LeafNodeCreator(1));
            children_is_leaf.push_back(true);
            continue;
        }
        int readTimes = 0;
        int writeTimes = 0;
        for(int j = 0; j < perSubDataset[i].size(); j++)
        {
            pair<int, int> tmp = (cntTree.find(perSubDataset[i][j].first))->second;
            readTimes += tmp.first;
            writeTimes += tmp.second;
        }
        // choose leaf node type, 0:array, 1:gapped array
        cout<<"Leaf node "<<i<<":\t"<<"read:"<<readTimes<<"\twrite:"<<writeTimes<<"\t:"<<float(readTimes) / (readTimes + writeTimes)<<endl;
        if(float(readTimes) / (readTimes + writeTimes) < kReadWriteRate)
        {
            cout<<"Leaf node "<<i<<"\t is gapped array!"<<endl;
            children.push_back(LeafNodeCreator(1));
        }
        else
        {
            cout<<"Leaf node "<<i<<"\t is array!"<<endl;
            children.push_back(LeafNodeCreator(0));
        }
        children_is_leaf.push_back(true);
        ((BasicLeafNode *)children[i])->SetDataset(perSubInit[i]);
    }
    cout << "End train" << endl;
}

void BasicInnerNode::Release()
{
    vector<void *>().swap(children);
    vector<bool>().swap(children_is_leaf);
    delete model;
}
#endif