#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#include <vector>
#include <algorithm>
#include "../params.h"
#include "../trainModel/model.h"
#include "../trainModel/lr.h"
#include "../trainModel/nn.h"
#include "../../cpp-btree/btree_map.h"
using namespace std;

class BasicLeafNode
{
public:
    BasicLeafNode()
    {
        isLeafNode = true;
        m_datasetSize = 0;
        model = new LinearRegression();
    }
    BasicLeafNode(params p)
    {
        parameter = p;
        isLeafNode = true;
        m_datasetSize = 0;
        model = new LinearRegression();
    }
    bool isLeaf() { return isLeafNode; }
    int getSize() { return m_datasetSize; }
    void getDataset(vector<pair<double, double>> &dataset)
    {
        for (int i = 0; i < m_dataset.size(); i++)
        {
            if (m_dataset[i].first != -1)
                dataset.push_back(m_dataset[i]);
        }
    }

    void chooseModel(int cnt)
    {
        if (cnt == 0)
            model = new LinearRegression();
        else if (cnt == 1)
            model = new Net();
    }

    virtual void train(const vector<pair<double, double>> &dataset){};

    virtual pair<double, double> find(double key){};
    virtual bool insert(pair<double, double> data){};
    virtual bool del(double key){};
    virtual bool update(pair<double, double> data){};

protected:
    bool isLeafNode;

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    params parameter;
    BasicModel *model;
};

#endif