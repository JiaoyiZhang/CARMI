#ifndef PIECEWISE_LR_H
#define PIECEWISE_LR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "../params.h"
#include "model.h"
#include <math.h>
using namespace std;

extern const int kMaxEpoch;
extern const double kLearningRate;
extern int kNeuronNumber;

class PiecewiseLR : public BasicModel
{
public:
    PiecewiseLR()
    {
        for(int i=0; i<kNeuronNumber; i++)
            theta.push_back({0.0001, 0.666});
    }
    
    void Train(const vector<pair<double, double> > &dataset, int len);
    int Predict(double key)
    {
        int s = 0, e = kNeuronNumber - 1;
        int mid;
        while (s < e)
        {
            mid = (s + e) / 2;
            if (point[mid] < key)
                s = mid + 1;
            else
                e = mid;
        }
        // return the predicted idx in the children
        int p = theta[e].first * key + theta[e].second;
        if(p < 0)
            p = 0;
        else if(p > length)
            p = length;
        return p;
    }

private:
    int length;
    vector<pair<double, double>> theta;
    vector<double> point;
};

void PiecewiseLR::Train(const vector<pair<double, double> > &dataset, int len)
{
    length = len - 1;
    int actualSize = 0;
    vector<double> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
            actualSize++;
        index.push_back(double(i) / double(dataset.size()));
    }
    if (actualSize == 0 || actualSize < 10)
    {
        return;
    }
    int seg = dataset.size() / kNeuronNumber;
    int i = 0;
    for(int k=1;k<=kNeuronNumber;k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        int end = min(k * seg, int(dataset.size() - 1));
        if(dataset[end].first != -1)
            point.push_back(dataset[end].first);
        else
            point.push_back(dataset[end-1].first);
        for (; i < end; i++)
        {
            if (dataset[i].first != -1)
            {
                t1 += dataset[i].first * dataset[i].first;
                t2 += dataset[i].first;
                t3 += dataset[i].first * index[i];
                t4 += index[i];
            }
        }
        auto theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
        auto theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
        theta1 *= len;
        theta2 *= len;
        theta[k-1] = {theta1, theta2};
    }
}
#endif