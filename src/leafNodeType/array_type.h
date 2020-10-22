#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H


#include "../params.h"
#include "trainModel/binary_search_model.h"
#include "trainModel/histogram.h"
#include "trainModel/lr.h"
#include "trainModel/nn.h"
#include <vector>
using namespace std;

extern vector<void *> index;  // store the entire index

class ArrayType
{
public:
    ArrayType(){};
    ArrayType(int maxNumber)
    {
        m_datasetSize = 0;
        error = 0;
        m_maxNumber = maxNumber;
        writeTimes = 0;
    }
    void SetDataset(const vector<pair<double, double>> &dataset);

    vector<pair<double, double>> m_dataset;
    LinearRegression model;
    int m_datasetSize;
    int error;

    int m_maxNumber; 
    int writeTimes;
};


void ArrayType::SetDataset(const vector<pair<double, double>> &dataset)
{
    m_dataset = dataset;
    m_datasetSize = m_dataset.size();
    if (m_datasetSize == 0)
        return;

    model.Train(m_dataset);
    for (int i = 0; i < m_datasetSize; i++)
    {
        double p = model.Predict(m_dataset[i].first);
        int preIdx = static_cast<int>(p * (m_datasetSize - 1));
        int e = abs(i - preIdx);
        if (e > error)
            error = e;
    }
    error++;
    writeTimes = 0;
}
#endif