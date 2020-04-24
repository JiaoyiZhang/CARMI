#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "../params.h"
using namespace std;

class BasicModel
{
public:
    BasicModel(){};

    virtual void train(vector<pair<double, double>> dataset, params param){};
    virtual double predict(double key){};
};

#endif