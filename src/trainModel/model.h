#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include "../params.h"
using namespace std;

class BasicModel
{
public:
    BasicModel(){};

    virtual void Train(vector<pair<double, double>> dataset, params param){};
    virtual double Predict(double key){};
};

#endif