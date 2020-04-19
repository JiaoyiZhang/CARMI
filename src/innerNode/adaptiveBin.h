#ifndef ADAPTIVE_BIN_H
#define ADAPTIVE_BIN_H

#include "innerNode.h"

template <typename lowerType>
class adaptiveBin : public basicInnerNode
{
public:
    adaptiveBin() : basicInnerNode(){};
    adaptiveBin(params secondStageParams, int maxKey, int childNum, int cap) : basicInnerNode(childNum)
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

    vector<double> index;
    params m_secondStageParams; // parameters of lower nodes
};

template <typename lowerType>
void adaptiveBin<lowerType>::init(const vector<pair<double, double>> &dataset)
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
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            adaptiveBin *child = new adaptiveBin(m_secondStageParams, maxKeyNum, childNumber, capacity);
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
pair<double, double> adaptiveBin<lowerType>::find(double key)
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
        return ((adaptiveBin *)children[mid])->find(key);
    return children[mid]->find(key);
}
template <typename lowerType>
bool adaptiveBin<lowerType>::update(pair<double, double> data)
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
        return ((adaptiveBin *)children[mid])->update(data);
    return children[mid]->update(data);
}

template <typename lowerType>
bool adaptiveBin<lowerType>::del(double key)
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
        return ((adaptiveBin *)children[mid])->del(key);
    return children[mid]->del(key);
}

template <typename lowerType>
bool adaptiveBin<lowerType>::insert(pair<double, double> data)
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
    int size = children[mid]->getSize();

    // if an insert will push a leaf node's
    // data structure over its maximum bound number of keys,
    // then we split the leaf data node
    if (children[mid]->isLeaf() && size >= maxKeyNum)
    {
        // The corresponding leaf level model in RMI
        // now becomes an inner level model
        adaptiveBin *newNode = new adaptiveBin(m_secondStageParams, maxKeyNum, childNumber, capacity);
        vector<pair<double, double>> dataset;
        children[mid]->getDataset(dataset);

        std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });

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
        if (newNode->index.size() == childNumber - 1)
        {
            perSubDataset.push_back(tmp);
            newNode->index.push_back(dataset[dataset.size() - 1].first);
        }

        // Each of the children leaf nodes trains its own
        // model on its portion of the data.
        for (int i = 0; i < childNumber; i++)
        {
            newNode->children[i]->train(perSubDataset[i]);
        }
        children[mid] = (lowerType *)newNode;
        return ((adaptiveBin *)children[mid])->insert(data);
    }
    else if (children[mid]->isLeaf() == false)
        return ((adaptiveBin *)children[mid])->insert(data);
    return children[mid]->insert(data);
}

template <typename lowerType>
long double adaptiveBin<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
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
        if (perSubDataset[i].size() > maxKeyNum)
            totalCost += adaptiveBin<lowerType>::getCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif