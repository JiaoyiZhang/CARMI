#ifndef LEAF_NODE_H
#define LEAF_NODE_H

#if kSearchMethod == 0
    #define SEARCH_METHOD(key, preIdx, start, end) BinarySearch(key, preIdx, start, end)
#else
    #define SEARCH_METHOD(key, preIdx, start, end) ExponentialSearch(key, preIdx, start, end)
#endif

#include <vector>
#include <algorithm>
#include <float.h>
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
        m_datasetSize = 0;
        maxPositiveError = 0;
        maxNegativeError = 0;
        model = new LinearRegression();
    }
    int GetSize() { return m_datasetSize; }
    void GetDataset(vector<pair<double, double>> *dataset)
    {
        for (int i = 0; i < m_dataset.size(); i++)
        {
            if (m_dataset[i].first != -1 && m_dataset[i].second != DBL_MIN)
                dataset->push_back(m_dataset[i]);
        }
    }

    virtual void SetDataset(const vector<pair<double, double>> &dataset) = 0;

    virtual pair<double, double> Find(double key) = 0;
    virtual bool Insert(pair<double, double> data) = 0;
    virtual bool Delete(double key) = 0;
    virtual bool Update(pair<double, double> data) = 0;

protected:
    vector<pair<double, double>> m_dataset;
    int m_datasetSize;

    BasicModel *model;

    int maxPositiveError;
    int maxNegativeError;
};

#endif