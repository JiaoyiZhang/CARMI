#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "../params.h"
using namespace std;

class basicModel
{
public:
    basicModel(){};

    virtual void train(vector<pair<double, double>> dataset, params param){};
    virtual double predict(double key){};
};

#endif