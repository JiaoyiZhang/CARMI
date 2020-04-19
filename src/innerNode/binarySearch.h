#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

#include "innerNode.h"

template <typename lowerType>
class binarySearchNode : public basicInnerNode
{
public:
    binarySearchNode() : basicInnerNode(){};
    binarySearchNode(params secondStageParams, int threshold, int childNum, int maxInsertNumber) : basicInnerNode(childNum)
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
    vector<double> index;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void binarySearchNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });

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
    {
        // cout << "second stage " << i << "    datasetSize is:" << perSubDataset[i].size() << endl;
        children[i]->train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

template <typename lowerType>
pair<double, double> binarySearchNode<lowerType>::find(double key)
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
    if (children[mid]->isLeaf() == false)
        return (children[mid])->find(key);
    return children[mid]->find(key);
}
template <typename lowerType>
bool binarySearchNode<lowerType>::update(pair<double, double> data)
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
    if (children[mid]->isLeaf() == false)
        return (children[mid])->update(data);
    return children[mid]->update(data);
}

template <typename lowerType>
bool binarySearchNode<lowerType>::del(double key)
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
    if (children[mid]->isLeaf() == false)
        return (children[mid])->del(key);
    return children[mid]->del(key);
}

template <typename lowerType>
bool binarySearchNode<lowerType>::insert(pair<double, double> data)
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
    if (children[mid]->isLeaf() == false)
        return (children[mid])->insert(data);
    return children[mid]->insert(data);
}

template <typename lowerType>
long double binarySearchNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = (log(childNum) / log(2)) * dataset.size();
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
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
    if (index.size() == childNum - 1)
    {
        perSubDataset.push_back(tmp);
    }

    // then iterate through the partitions in sorted order
    for (int i = 0; i < childNum; i++)
    {
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif