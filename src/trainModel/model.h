#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "../params.h"
using namespace std;

class BasicModel
{
public:
    BasicModel(){};

    virtual void Train(const vector<pair<double, double>> &dataset, const params param) = 0;
    virtual double Predict(double key) = 0;
};

#endif