#ifndef BINARY_SEARCH_MODEL_H
#define BINARY_SEARCH_MODEL_H

#include "model.h"

class BinarySearchModel : public BasicModel
{
public:
    BinarySearchModel(int childNum) { childNumber = childNum; }
    void Train(const vector<pair<double, double>> &dataset);
    double Predict(double key);

private:
    vector<double> index;
    int childNumber;
};

void BinarySearchModel::Train(const vector<pair<double, double>> &dataset)
{
    if (dataset.size() == 0)
        return;
    int value = dataset.size() / childNumber;
    for (int i = value - 1; i < dataset.size(); i += value)
    {
        if (dataset[i].first != -1)
        {
            index.push_back(dataset[i].first);
        }
        else
        {
            for (int j = i + 1; j < dataset.size(); j++)
            {
                if (dataset[j].first != -1)
                {
                    index.push_back(dataset[j].first);
                    break;
                }
            }
        }
    }
    if (index.size() == childNumber - 1)
    {
        if (dataset[dataset.size() - 1].first != -1)
        {
            index.push_back(dataset[dataset.size() - 1].first);
        }
        else
        {
            for (int j = dataset.size() - 2; j >= 0; j--)
            {
                if (dataset[j].first != -1)
                {
                    index.push_back(dataset[j].first);
                    break;
                }
            }
        }
    }
}

double BinarySearchModel::Predict(double key)
{
    int start_idx = 0;
    int end_idx = childNumber - 1;
    int mid;
    while (start_idx < end_idx)
    {
        mid = (start_idx + end_idx) / 2;
        if (index[mid] < key)
            start_idx = mid + 1;
        else
            end_idx = mid;
    }
    double p = float(mid) / childNumber;
    p = p < 0 ? 0 : p;
    p = p > 1 ? 1 : p;
    return p;
}

#endif