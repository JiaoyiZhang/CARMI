#ifndef LR_H
#define LR_H
#include "params.h"
#include <iostream>
#include <vector>
using namespace std;

class linearRegression
{
public:
    linearRegression(){};
    void train(vector<pair<double, double>> dataset, params param);
    double predict(double key);

private:
    double theta1;
    double theta2;
};

void linearRegression::train(vector<pair<double, double>> dataset, params param)
{
    if (dataset.size() == 0)
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
            double p = theta1 * dataset[j].first + theta2;
            p = (p > 1 ? 1 : p);
            p = (p < 0 ? 0 : p);
            error1 += (p - j) * dataset[j].first;
            error2 += p - j;
        }
        theta1 = theta1 - param.learningRate * error1 / dataset.size();
        theta2 = theta2 - param.learningRate * error2 / dataset.size();

        double loss = 0.0;
        for (int j = 0; j < dataset.size(); j++)
        {
            double p = theta1 * dataset[j].first + theta2;
            p = (p > 1 ? 1 : p);
            p = (p < 0 ? 0 : p);
            loss += (p - j) * (p - j);
        }
        loss = loss / (dataset.size() * 2);
        // cout << "iteration: " << i << "    loss is: " << loss << endl;
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