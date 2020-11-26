#ifndef GA_TYPE_H
#define GA_TYPE_H

#include "../params.h"
#include "../trainModel/lr.h"
#include <float.h>
#include <vector>
using namespace std;

extern pair<double, double> *entireData;
extern int *mark;
extern int entireDataSize;
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
        m_left = -1;
    }
    void SetDataset(const vector<pair<double, double>> &dataset, int cap);

    LinearRegression model;
    int m_left;        // the left boundary of the leaf node in the global array
    int m_datasetSize; // the current amount of data
    int capacity;      // the current maximum capacity of the leaf node

    int maxIndex;   // tht index of the last one
    int error;      // the boundary of binary search
    double density; // the maximum density of the leaf node data
};

void GappedArrayType::SetDataset(const vector<pair<double, double>> &subDataset, int cap)
{
    capacity = cap;
    while ((float(subDataset.size()) / float(capacity) > density))
    {
        int newSize = capacity / density;
        capacity = newSize;
    }
    m_datasetSize = 0;

    int k = density / (1 - density);
    int cnt = 0;
    vector<pair<double, double>> newDataset(capacity, pair<double, double>{-1, -1});
    int j = 0;
    for (int i = 0; i < subDataset.size(); i++)
    {
        if ((subDataset[i].first != -1) && (subDataset[i].second != DBL_MIN))
        {
            cnt++;
            if (cnt > k)
            {
                j++;
                cnt = 0;
            }
            newDataset[j++] = subDataset[i];
            maxIndex = j - 1;
            m_datasetSize++;
        }
    }

    for (int i = m_left, j = 0; j < capacity; i++, j++)
        entireData[i] = newDataset[j];

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