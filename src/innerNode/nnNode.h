#ifndef NN_NODE_H
#define NN_NODE_H

#include "innerNode.h"
#include "../trainModel/nn.h"

template <typename lowerType>
class nnNode : public basicInnerNode
{
public:
    nnNode() : basicInnerNode(){};
    nnNode(params firstStageParams, params secondStageParams, int threshold, int childNum, int maxInsertNumber) : basicInnerNode(childNum)
    {
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;

        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
        }
    }

    void init(const vector<pair<double, double>> &dataset);

    pair<double, double> find(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->find(key);
    }
    bool insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->insert(data);
    }
    bool del(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->del(key);
    }
    bool update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return children[preIdx]->update(data);
    }

    static long double getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

private:
    params m_firstStageParams;       // parameters of network
    params m_secondStageParams;      // parameters of lower nodes
    net m_firstStageNetwork = net(); // network of the first stage
};

template <typename lowerType>
void nnNode<lowerType>::init(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    // std::sort(dataset.begin(), dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });

    cout << "train first stage" << endl;
    m_firstStageNetwork.train(dataset, m_firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }

    for (int i = 0; i < dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() == dataset.size())
            return init(dataset);
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
    {
        // cout << "second stage " << i << "    datasetSize is:" << perSubDataset[i].size() << endl;
        children[i]->train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

template <typename lowerType>
long double nnNode<lowerType>::getCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double initCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tinitCost is:" << initCost << endl;
    long double totalCost = initCost;
    // cout << " DatasetSize is : " << dataset.size() << endl;
    if (dataset.size() == 0)
        return 0;

    net tmpNet = net();
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    tmpNet.train(dataset, firstStageParams);
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < childNumber; i++)
    {
        perSubDataset.push_back(tmp);
    }
    for (int i = 0; i < dataset.size(); i++)
    {
        double p = tmpNet.predict(dataset[i].first);
        p = p * (childNumber - 1);
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    
    for (int i = 0; i < childNum; i++)
    {
        totalCost += lowerType::getCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

#endif