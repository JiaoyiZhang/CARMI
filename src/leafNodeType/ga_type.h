#ifndef GA_TYPE_H
#define GA_TYPE_H


#include "../params.h"
#include "../trainModel/lr.h"
#include <float.h>
#include <vector>
using namespace std;

extern vector<vector<pair<double, double>>> entireDataset;

class GappedArrayType
{
public:
    GappedArrayType(){};
    GappedArrayType(int cap)
    {
        m_datasetSize = 0;
        error = 0;
        density = kDensity;
        capacity = cap;
        maxIndex = -2;
        datasetIndex = -1;
    }
    void SetDataset(const vector<pair<double, double> > &dataset);

    int datasetIndex; // index in the dataset (vector<vector<>>)
    LinearRegression model;
    int m_datasetSize;
    int error;

    int maxIndex;   // tht index of the last one
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
};

void GappedArrayType::SetDataset(const vector<pair<double, double> > &subDataset)
{
    while ((float(subDataset.size()) / float(capacity) > density))
    {
        int newSize = capacity / density;
        capacity = newSize;
    }
    m_datasetSize = 0;

    vector<pair<double, double> > newDataset(capacity, pair<double, double>{-1, -1});
    maxIndex = -2;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if ((subDataset[i].first != -1) && (subDataset[i].second != DBL_MIN))
        {
            maxIndex += 2;
            newDataset[maxIndex] = subDataset[i];
            m_datasetSize++;
        }
    }
    entireDataset.push_back(newDataset);
    datasetIndex = entireDataset.size() - 1;
    model.Train(newDataset, capacity);
    for (int i = 0; i < newDataset.size(); i++)
    {
        if (newDataset[i].first != -1)
        {
            int p = model.Predict(newDataset[i].first);
            int e = abs(i - p);
            if (e > error)
                error = e;
        }
    }
    error++;
}
#endif