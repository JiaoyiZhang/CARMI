#ifndef HistogramModel_H
#define HistogramModel_H

#include "model.h"

class HistogramModel : public BasicModel
{
public:
    HistogramModel(int childNum)
    {
        childNumber = childNum;
        value = 1;
        for (int i = 0; i < 100; i++)
            table.push_back(0);
    }
    void Train(const vector<pair<double, double>> &dataset);
    double Predict(double key);

    // designed for test
    float GetValue(){return value;}
    void GetTable(vector<double> &t)
    {
        for(int i=0;i<table.size();i++)
            t.push_back(table[i]);
    }

private:
    float value;
    vector<double> table;
    int childNumber;
};

void HistogramModel::Train(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    double maxValue, minValue;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            minValue = dataset[i].first;
            break;
        }
    }
    for (int i = dataset.size() - 1; i >= 0; i--)
    {
        if (dataset[i].first != -1)
        {
            maxValue = dataset[i].first;
            break;
        }
    }
    value = float(maxValue - minValue) / float(childNumber);
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            int idx = float(dataset[i].first * 99.0) / (value * childNumber);
            table[idx]++;
        }
    }
    table[0] = float(table[0]) / dataset.size();
    for (int i = 1; i < 100; i++)
    {
        table[i] = table[i - 1] + float(table[i]) / dataset.size();
    }
}

double HistogramModel::Predict(double key)
{
    int idx = float(key * 99) / (value * childNumber);
    double p = table[idx];
    p = p < 0 ? 0 : p;
    p = p > 1 ? 1 : p;
    return p;
}

#endif