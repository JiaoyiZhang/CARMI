#ifndef ARRAY_TYPE_H
#define ARRAY_TYPE_H

#include "../params.h"
#include "../trainModel/lr.h"
#include <vector>
using namespace std;

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
        datasetIndex = -1;
    }
    void SetDataset(const vector<pair<double, double>> &dataset);

    int datasetIndex; // index in the dataset (vector<vector<>>)
    // vector<pair<double, double>> m_dataset;
    LinearRegression model; // 20 Byte
    int m_datasetSize;
    int error;

    int m_maxNumber;
    int writeTimes;
};

void ArrayType::SetDataset(const vector<pair<double, double>> &dataset)
{
    entireDataset.push_back(dataset);
    datasetIndex = entireDataset.size() - 1;
    m_datasetSize = dataset.size();
    if (m_datasetSize == 0)
        return;

    model.Train(dataset, m_datasetSize);
    int sum = 0;
    for (int i = 0; i < m_datasetSize; i++)
    {
        int p = model.Predict(dataset[i].first);
        int e = abs(i - p);
        sum += e;
    }
    error = float(sum) / m_datasetSize + 1;
    writeTimes = 0;
}
#endif