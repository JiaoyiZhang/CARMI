#ifndef DivisionModel_H
#define DivisionModel_H

#include "model.h"

class DivisionModel : public BasicModel
{
public:
    DivisionModel(int childNum) { childNumber = childNum; }
    void Train(const vector<pair<double, double>> &dataset);
    double Predict(double key);

private:
    float value;
    int childNumber;
};

void DivisionModel::Train(const vector<pair<double, double>> &dataset)
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
}

double DivisionModel::Predict(double key)
{
    double p = float(key) / (value * childNumber);
    p = p < 0 ? 0 : p;
    p = p > 1 ? 1 : p;
    return p;
}

#endif