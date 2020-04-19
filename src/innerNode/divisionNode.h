#ifndef DIVISION_NODE_H
#define DIVISION_NODE_H

#include "innerNode.h"

template <typename lowerType>
class divisionNode : public basicInnerNode
{
public:
    divisionNode() : basicInnerNode(){};
    divisionNode(params secondStageParams, int threshold, int childNum, int maxInsertNumber) : basicInnerNode(childNum)
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

private:
    float value;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void divisionNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });
    cout << "train first stage" << endl;
    double maxValue = dataset[dataset.size() - 1].first;
    double minValue = dataset[0].first;
    value = float(maxValue - minValue) / float(childNumber);
    // cout << "maxValue is: " << maxValue << "\tminValue: " << minValue << "\tvalue:" << value << endl;

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
        // cout << "second stage " << i << "    datasetSize is:" << perSubDataset[i].size() << endl;
        children[i]->train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> divisionNode<lowerType>::find(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return children[preIdx]->find(key);
    return children[preIdx]->find(key);
}
template <typename lowerType>
bool divisionNode<lowerType>::update(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return children[preIdx]->update(data);
    return children[preIdx]->update(data);
}

template <typename lowerType>
bool divisionNode<lowerType>::del(double key)
{
    int preIdx = key / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return children[preIdx]->del(key);
    return children[preIdx]->del(key);
}

template <typename lowerType>
bool divisionNode<lowerType>::insert(pair<double, double> data)
{
    int preIdx = data.first / value;
    if (preIdx >= childNumber)
        preIdx = childNumber - 1;
    else if (preIdx < 0)
        preIdx = 0;
    if (children[preIdx]->isLeaf() == false)
        return children[preIdx]->insert(data);
    return children[preIdx]->insert(data);
}

template <typename lowerType>
long double divisionNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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

    cout << "train next stage" << endl;
    for (int i = 0; i < childNumber; i++)
    {
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}
#endif