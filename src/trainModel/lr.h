#ifndef LR_H
#define LR_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include "../params.h"
#include "model.h"
using namespace std;

class LinearRegression : public BasicModel
{
public:
    LinearRegression()
    {
        theta1 = 0.0001;
        theta2 = 0.666;
    }
    void Train(const vector<pair<double, double>> &dataset);
    double Predict(double key);

    // designed for test
    double GetTheta1(){return theta1;}
    double GetTheta2(){return theta2;}

private:
    double theta1;
    double theta2;
};

void LinearRegression::Train(const vector<pair<double, double>> &dataset)
{
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
    double t1 = 0, t2 = 0, t3 = 0, t4 = 0;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
        {
            t1 += dataset[i].first * dataset[i].first;
            t2 += dataset[i].first;
            t3 += dataset[i].first * index[i];
            t4 += index[i];
        }
    }
    theta1 = (t3 * actualSize - t2 * t4) / (t1 * actualSize - t2 * t2);
    theta2 = (t1 * t4 - t2 * t3) / (t1 * actualSize - t2 * t2);
}

double LinearRegression::Predict(double key)
{
    double p = theta1 * key + theta2;
    p = p < 0 ? 0 : p;
    p = p > 1 ? 1 : p;
    return p;
}
#endif