#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "../params.h"
using namespace std;

class BasicModel
{
public:
    BasicModel(){};

    virtual void Train(const vector<pair<double, double>> &dataset, int len) = 0;
    virtual int Predict(double key) = 0;
};

#endif