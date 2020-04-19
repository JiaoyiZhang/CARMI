#ifndef ADAPTIVE_DIV_H
#define ADAPTIVE_DIV_H

#include "innerNode.h"

template <typename lowerType>
class adaptiveDiv : public basicInnerNode
{
public:
    adaptiveDiv() : basicInnerNode(){};
    adaptiveDiv(params secondStageParams, int maxKey, int childNum, int cap) : basicInnerNode(childNum)
    {
        m_secondStageParams = secondStageParams;

        maxKeyNum = maxKey;
        density = 0.75;
        capacity = cap;
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
    int maxKeyNum;  // the maximum amount of data

    double value;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void adaptiveDiv<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });

    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    value = float(maxValue - minValue) / float(childNumber);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / value;
        preIdx = min(preIdx, childNumber - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    cout << "train next stage" << endl;
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            adaptiveDiv *child = new adaptiveDiv(m_secondStageParams, maxKeyNum, childNumber, capacity);
            child->init(perSubDataset[i]);
            children.push_back((lowerType *)child);
        }
        else
        {
            // Otherwise, the partition is under the maximum bound number of keys,
            // so we could just make this partition a leaf node
            lowerType *child = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            child->train(perSubDataset[i]);
            children.push_back(child);
        }
    }
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> adaptiveDiv<lowerType>::find(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveDiv *)children[preIdx])->find(key);
    return children[preIdx]->find(key);
}
template <typename lowerType>
bool adaptiveDiv<lowerType>::update(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveDiv *)children[preIdx])->update(data);
    return children[preIdx]->update(data);
}

template <typename lowerType>
bool adaptiveDiv<lowerType>::del(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return ((adaptiveDiv *)children[preIdx])->del(key);
    return children[preIdx]->del(key);
}

template <typename lowerType>
bool adaptiveDiv<lowerType>::insert(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;

    int size = children[preIdx]->getSize();
    
    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (children[preIdx]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        adaptiveDiv *newNode = new adaptiveDiv(m_secondStageParams, maxKeyNum, childNumber, capacity);
        vector<pair<double, double>> dataset;
        children[preIdx]->getDataset(dataset);

        std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
        double maxValue = dataset[dataset.size() - 1].first;
        double minValue = dataset[0].first;
        value = float(maxValue - minValue) / float(childNumber);

        // a number of children leaf level models are created
        for (int i = 0; i < childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            newNode->children.push_back(temp);
        }

        // The data from the original leaf node is then
        // distributed to the newly created children leaf nodes
        // according to the original nodeâ€™s model.
        vector<vector<pair<double, double>>> perSubDataset;
        vector<pair<double, double>> temp;
        for (int i = 0; i < childNumber; i++)
        {
            perSubDataset.push_back(temp);
        }
        for (int i = 0; i < dataset.size(); i++)
        {
            int pIdx = data.first / value;
            perSubDataset[pIdx].push_back(dataset[i]);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < childNumber; i++)
        {
            newNode->children[i]->train(perSubDataset[i]);
        }
        children[preIdx] = (lowerType *)newNode;
        return ((adaptiveDiv *)children[preIdx])->insert(data);
    }
    else if (children[preIdx]->isLeaf() == false)
        return ((adaptiveDiv *)children[preIdx])->insert(data);
    return children[preIdx]->insert(data);
}

template <typename lowerType>
long double adaptiveDiv<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    // cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return 0;

    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    float value = float(maxValue - minValue) / float(childNumber);

    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        int preIdx = float(dataset[i].first) / value;
        preIdx = min(preIdx, childNumber - 1);
        perSubDataset[preIdx].push_back(dataset[i]);
    }

    for (int i = 0; i < childNum; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            totalCost += adaptiveBin<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        }
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}
#endif