#ifndef GAPPED_ARRAY_H
#define GAPPED_ARRAY_H

#include <iostream>
#include <vector>
#include <math.h>
using namespace std;

class gappedArray
{
public:
    gappedArray(const vector<pair<double, double>> &dataset, int maxNum)
    {
        maxKeyNum = 0;
        m_dataset = dataset;
        maxKeyNum = maxNum;
        childNumber = ceil((1.0 * m_datasetSize) / (1 * maxKeyNum));
        m_datasetSize = m_dataset.size();
        if (m_datasetSize > maxKeyNum)
            isLeafNode = false;
        else
            isLeafNode = true;

        train();
    }
    bool insert();
    pair<double, double> find(double key);

private:
    void expand();
    void train();

    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
    int maxKeyNum;
    int childNumber;
    bool isLeafNode;

    double theta1;
    double theta2;

    vector<gappedArray> children;
};

void gappedArray::train()
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    for (int i = 0; i < 5000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = m_dataset[j].second;
            error2 += p - y;
            error1 += (p - y) * m_dataset[j].first;
        }
        theta1 = theta1 - 0.000001 * error1 / m_datasetSize;
        theta2 = theta2 - 0.000001 * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = theta1 * m_dataset[j].first + theta2;
            p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
            p = (p < 0 ? 0 : p);
            double y = m_dataset[j].second;
            loss += (p - y) * (p - y);
        }
        loss = loss / (m_datasetSize * 2);
    }
    if (!isLeafNode)
    {
        vector<pair<double, double>> nextStageDataset;
        int nowNum = 0;
        for (int i = 0; i < m_datasetSize; i++)
        {
            double p = theta1 * m_dataset[i].first + theta2;
            p = (p > m_datasetSize - 1 ? m_datasetSize - 1 : p);
            p = (p < 0 ? 0 : p);
            nextStageDataset.push_back(m_dataset[i]);
            if (int(p / maxKeyNum) > nowNum)
            {
                gappedArray ga = gappedArray(nextStageDataset, maxKeyNum);
                children.push_back(ga);
                nextStageDataset.clear();
                nowNum++;
            }
        }
        m_dataset.clear();
    }
}

#endif