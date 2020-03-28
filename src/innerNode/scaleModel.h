#ifndef SCALE_MODEL_H
#define SCALE_MODEL_H

#include "../params.h"
#include <vector>
using namespace std;

template <typename lowerType>
class scaleModel
{
public:
    scaleModel(){};
    scaleModel(params secondStageParams, int maxKey, int splitChildNumber, int cap)
    {
        m_secondStageParams = secondStageParams;

        childNumber = splitChildNumber;
        maxKeyNum = maxKey;
        capacity = cap;
        isLeafNode = false;
    }

    void initialize(vector<pair<double, double>> &dataset);
    bool isLeaf() { return isLeafNode; }

    pair<double, double> find(double key);
    bool insert(pair<double, double> data);
    bool del(double key);
    bool update(pair<double, double> data);

private:
    vector<double> index;

    vector<lowerType *> children; // The type of child node may be innerNode or leafNode
    int childNumber;              // the number of children

    int capacity;  // the current maximum capacity of the leaf node data
    int maxKeyNum; // the maximum amount of data

    params m_secondStageParams; // parameters of lower nodes

    bool isLeafNode;
};

template <typename lowerType>
void scaleModel<lowerType>::initialize(vector<pair<double, double>> &dataset)
{
    cout << "Start initialize!" << endl;
    // cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return;
    std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });

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

    // then iterate through the partitions in sorted order
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() > maxKeyNum)
        {
            // If a partition has more than the maximum bound number of
            // keys, then this partition is oversized,
            // so we create a new inner node and
            // recursively call Initialize on the new node.
            scaleModel *child = new scaleModel(m_secondStageParams, maxKeyNum, childNumber, capacity);
            child->initialize(perSubDataset[i]);
            children.push_back((lowerType *)child);
        }
        else
        {
            // make this partition a leaf node
            // cout << i << ": leaf node! Dataset size is:" << perSubDataset[i].size() << endl;
            lowerType *child = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            child->train(perSubDataset[i]);
            children.push_back(child);
        }
    }
    cout << "Finish" << endl;
    return;
}

template <typename lowerType>
pair<double, double> scaleModel<lowerType>::find(double key)
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
        return ((scaleModel *)children[mid])->find(key);
    return children[mid]->find(key);
}

template <typename lowerType>
bool scaleModel<lowerType>::update(pair<double, double> data)
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
        return ((scaleModel *)children[mid])->update(data);
    return children[mid]->update(data);
}

template <typename lowerType>
bool scaleModel<lowerType>::del(double key)
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
        return ((scaleModel *)children[mid])->del(key);
    return children[mid]->del(key);
}

template <typename lowerType>
bool scaleModel<lowerType>::insert(pair<double, double> data)
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
    if (children[mid]->isLeaf() && size >= maxKeyNum)
    {
        scaleModel *newNode = new scaleModel(m_secondStageParams, maxKeyNum, childNumber, capacity);
        vector<pair<double, double>> dataset;
        children[mid]->getDataset(dataset);
        vector<vector<pair<double, double>>> perSubDataset;
        vector<pair<double, double>> tmp;
        for (int i = 0; i < dataset.size(); i++)
        {
            tmp.push_back(dataset[i]);
            if ((i + 1) % (dataset.size() / childNumber) == 0)
            {
                perSubDataset.push_back(tmp);
                newNode->index.push_back(dataset[i].first);
                tmp = vector<pair<double, double>>();
            }
        }
        if (index.size() != childNumber)
        {
            perSubDataset.push_back(tmp);
            newNode->index.push_back(dataset[dataset.size() - 1].first);
        }

        // a number of children leaf level models are created
        for (int i = 0; i < childNumber; i++)
        {
            lowerType *temp = new lowerType(maxKeyNum, m_secondStageParams, capacity);
            newNode->children.push_back(temp);
        }

        for (int i = 0; i < childNumber; i++)
        {
            newNode->children[i]->train(perSubDataset[i]);
        }
        children[mid] = (lowerType *)newNode;
        return ((scaleModel *)children[mid])->insert(data);
    }
    else if (children[mid]->isLeaf() == false)
        return ((scaleModel *)children[mid])->insert(data);
    return children[mid]->insert(data);
}
#endif