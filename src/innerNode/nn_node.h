#ifndef NN_NODE_H
#define NN_NODE_H

#include "inner_node.h"
#include "../trainModel/nn.h"
const int LOWER_ID1 = 1;
const int UPPER_ID1 = 1;
#define LOWER_TYPE1 GappedArray
#define UPPER_TYPE1 AdaptiveNN

class NetworkNode : public BasicInnerNode
{
public:
    NetworkNode() : BasicInnerNode(){};
    NetworkNode(int childNum) : BasicInnerNode(childNum){};
    NetworkNode(int threshold, int childNum, int maxInsertNumber) : BasicInnerNode(childNum)
    {
        for (int i = 0; i < childNumber; i++)
        {
            children.push_back(LeafNodeCreator(LOWER_ID1, threshold, maxInsertNumber));
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
        return ((BasicLeafNode *)children[preIdx])->Find(key);
    }
    bool Insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        return ((BasicLeafNode *)children[preIdx])->Insert(data);
    }
    bool Delete(double key)
    {
        double p = m_firstStageNetwork.Predict(key);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Delete(key);
        return ((BasicLeafNode *)children[preIdx])->Delete(key);
    }
    bool Update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.Predict(data.first);
        int preIdx = static_cast<int>(p * (childNumber - 1));
        if (children_is_leaf[preIdx] == false)
            return ((BasicInnerNode *)children[preIdx])->Update(data);
        return ((BasicLeafNode *)children[preIdx])->Update(data);
    }

    static long double GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum);

protected:
    Net m_firstStageNetwork = Net(); // network of the first stage
};

void NetworkNode::Initialize(const vector<pair<double, double>> &dataset)
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
        int preIdx = static_cast<int>(p * (childNumber - 1));
        perSubDataset[preIdx].push_back(dataset[i]);
    }
    for (int i = 0; i < childNumber; i++)
    {
        if (perSubDataset[i].size() == dataset.size() / 2)
            return Initialize(dataset);
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < childNumber; i++)
        ((BasicLeafNode *)children[i])->SetDataset(perSubDataset[i]);
    cout << "End train" << endl;
}

long double NetworkNode::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    Net tmpNet = Net();
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
        totalCost += LOWER_TYPE1::GetCost(cntTree, perSubDataset[i]);
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}

class AdaptiveNN : public NetworkNode
{
public:
    AdaptiveNN() : NetworkNode(){};
    AdaptiveNN(int maxKey, int childNum, int cap) : NetworkNode(childNum)
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

void AdaptiveNN::Initialize(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;

    cout << "train first stage" << endl;
    this->m_firstStageNetwork.Train(dataset);
    for (int i = 0; i < childNumber; i++)
    {
        children.push_back(LeafNodeCreator(LOWER_ID1, maxKeyNum, capacity));
        children_is_leaf.push_back(true);
    }

    auto tmpDataset = dataset;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(tmpDataset.begin(), tmpDataset.end(), default_random_engine(seed));
    vector<vector<pair<double, double>>> perSubDataset;
    vector<pair<double, double>> tmp;
    for (int i = 0; i < this->childNumber; i++)
        perSubDataset.push_back(tmp);
    for (int i = 0; i < tmpDataset.size(); i++)
    {
        double p = this->m_firstStageNetwork.Predict(tmpDataset[i].first);
        int preIdx = static_cast<int>(p * (this->childNumber - 1));
        if (perSubDataset[preIdx].size() < 100)
            perSubDataset[preIdx].push_back(tmpDataset[i]);
        else if (perSubDataset[preIdx].size() == 100)
        {
            perSubDataset[preIdx].push_back(tmpDataset[i]);
            std::sort(perSubDataset[preIdx].begin(), perSubDataset[preIdx].end(), [](pair<double, double> p1, pair<double, double> p2) {
                return p1.first < p2.first;
            });
            ((BasicLeafNode *)this->children[preIdx])->SetDataset(perSubDataset[preIdx]);
        }
        else
            Insert(tmpDataset[i]);
    }
    cout << "End train" << endl;
}

bool AdaptiveNN::Insert(pair<double, double> data)
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
            UPPER_TYPE1 *newNode = (UPPER_TYPE1 *)InnerNodeCreator(UPPER_ID1, maxKeyNum, this->childNumber, capacity);
            vector<pair<double, double>> dataset;
            ((BasicLeafNode *)this->children[preIdx])->GetDataset(&dataset);
            newNode->m_firstStageNetwork.Train(dataset);

            // a number of children leaf level moDeletes are created
            for (int i = 0; i < this->childNumber; i++)
            {
                LOWER_TYPE1 *temp = (LOWER_TYPE1 *)LeafNodeCreator(LOWER_ID1, maxKeyNum, capacity);
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
    return ((BasicLeafNode *)this->children[preIdx])->Insert(data);
}

long double AdaptiveNN::GetCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, vector<pair<double, double>> &dataset, int cap, int maxNum)
{
    double InitializeCost = 16;
    cout << "child: " << childNum << "\tsize: " << dataset.size() << "\tInitializeCost is:" << InitializeCost << endl;
    long double totalCost = InitializeCost;
    if (dataset.size() == 0)
        return 0;

    Net tmpNet = Net();
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
            totalCost += UPPER_TYPE1::GetCost(cntTree, childNum, perSubDataset[i], cap, maxNum);
        else
            totalCost += LOWER_TYPE1::GetCost(cntTree, perSubDataset[i]);
    }
    cout << "sub tree get cost finish!" << endl;
    return totalCost;
}
#endif