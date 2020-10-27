#ifndef GA_TYPE_H
#define GA_TYPE_H


#include "../params.h"
#include "../trainModel/lr.h"
#include <vector>
using namespace std;


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
    }
    void SetDataset(const vector<pair<double, double>> &dataset);

    vector<pair<double, double>> m_dataset;
    LinearRegression model;
    int m_datasetSize;
    int error;

    int maxIndex;   // tht index of the last one
    int capacity;   // the current maximum capacity of the leaf node data
    double density; // the maximum density of the leaf node data
};

void GappedArrayType::SetDataset(const vector<pair<double, double>> &subDataset)
{
    while ((float(subDataset.size()) / float(capacity) > density))
    {
        int newSize = capacity / density;
        capacity = newSize;
    }
    m_datasetSize = 0;

    vector<pair<double, double>> newDataset(capacity, pair<double, double>{-1, -1});
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
    m_dataset = newDataset;
    model.Train(m_dataset);
    for (int i = 0; i < m_dataset.size(); i++)
    {
        if (m_dataset[i].first != -1)
        {
            double p = model.Predict(m_dataset[i].first);
            int preIdx = static_cast<int>(p * (capacity - 1));
            int e = abs(i - preIdx);
            if (e > error)
                error = e;
        }
    }
    error++;
}
#endif