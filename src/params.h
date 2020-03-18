#ifndef PARAMS
#define PARAMS

struct params
{
    params(double lr, int epoch, int num)
    {
        maxEpoch = epoch;
        learningRate = lr;
        neuronNumber = num;
    }

    params()
    {
    }

    int maxEpoch;
    double learningRate;
    int neuronNumber;
};

#endif