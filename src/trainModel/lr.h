#ifndef LR_H
#define LR_H
#include "../params.h"
#include <iostream>
#include <vector>
using namespace std;

class linearRegression
{
public:
    linearRegression()
    {
        theta1 = 0.0001;
        theta2 = 0.666;
    }
    void train(vector<pair<double, double>> dataset, params param);
    double predict(double key);

private:
    double theta1;
    double theta2;
};

void linearRegression::train(vector<pair<double, double>> dataset, params param)
{
    int actualSize = 0;
    vector<double> index;
    for (int i = 0; i < dataset.size(); i++)
    {
        if (dataset[i].first != -1)
            actualSize++;
        index.push_back(double(i) / double(dataset.size()));
    }
    if (actualSize == 0)
    {
        cout << "This node is empty!" << endl;
        return;
    }
    for (int i = 0; i < param.maxEpoch; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < dataset.size(); j++)
        {
            if (dataset[j].first != -1)
            {
                double p = theta1 * dataset[j].first + theta2;
                p = (p > 1 ? 1 : p);
                p = (p < 0 ? 0 : p);
                error1 += (p - index[j]) * dataset[j].first;
                error2 += p - index[j];
            }
        }
        theta1 = theta1 - param.learningRate * error1 / actualSize;
        theta2 = theta2 - param.learningRate * error2 / actualSize;

        double loss = 0.0;
        for (int j = 0; j < dataset.size(); j++)
        {
            if (dataset[j].first != -1)
            {
                double p = theta1 * dataset[j].first + theta2;
                p = (p > 1 ? 1 : p);
                p = (p < 0 ? 0 : p);
                loss += (p - index[j]) * (p - index[j]);
            }
        }
        loss = loss / (actualSize * 2);
    }
}

double linearRegression::predict(double key)
{
    double p = theta1 * key + theta2;
    p = p < 0 ? 0 : p;
    p = p > 1 ? 1 : p;
    return p;
}
#endif