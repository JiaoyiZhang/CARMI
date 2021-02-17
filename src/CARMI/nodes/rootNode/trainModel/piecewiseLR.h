#ifndef PIECEWISE_LR_H
#define PIECEWISE_LR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <math.h>
using namespace std;

class PiecewiseLR
{
public:
    PiecewiseLR()
    {
        for (int i = 0; i < 8; i++)
            theta.push_back({0.0001, 0.666});
    }

    void Train(const vector<pair<double, double>> &dataset, int len);
    int Predict(double key)
    {
        int s = 0;
        int e = 7;
        int mid;
        while (s < e)
        {
            mid = (s + e) / 2;
            if (point[mid].first < key)
                s = mid + 1;
            else
                e = mid;
        }
        // return the predicted idx in the children
        int p = theta[e].first * key + theta[e].second;
        if (e == 0)
        {
            if (p < 0)
                p = 0;
            else if (p > point[e].second)
                p = point[e].second;
        }
        else
        {
            if (p < point[e - 1].second)
                p = point[e - 1].second;
            else if (p > point[e].second)
                p = point[e].second;
        }
        return p;
    }

private:
    int length;
    vector<pair<double, double>> theta;
    vector<pair<double, int>> point; // <point.first, boundary>
};

void PiecewiseLR::Train(const vector<pair<double, double>> &dataset, int len)
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
    if (actualSize == 0)
        return;
        
    int seg = dataset.size() / 8;
    int i = 0;
    for (int k = 1; k <= 8; k++)
    {
        double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
        int end = min(k * seg, int(dataset.size() - 1));
        if (dataset[end].first != -1)
            point.push_back({dataset[end].first, length});
        else
            point.push_back({dataset[end - 1].first, length});
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
        theta[k - 1] = {theta1, theta2};
        int pointIdx = theta1 * point[k - 1].first + theta2;
        if (pointIdx < 0)
            pointIdx = 0;
        else if (pointIdx > length)
            pointIdx = length;
        point[k - 1].second = pointIdx;
    }
}
#endif