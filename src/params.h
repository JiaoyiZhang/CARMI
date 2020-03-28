#ifndef PARAMS
#define PARAMS

struct params
{
    params(double lr1, double lr2, int epoch, int num, double t1, double t2)
    {
        maxEpoch = epoch;
        learningRate1 = lr1;
        learningRate2 = lr2;
        neuronNumber = num;
        initTheta1 = t1;
        initTheta2 = t2;
    }

    params(double lr, int epoch, int num, double t1, double t2)
    {
        maxEpoch = epoch;
        learningRate1 = lr;
        learningRate2 = lr;
        neuronNumber = num;
        initTheta1 = t1;
        initTheta2 = t2;
    }

    params()
    {
    }

    int maxEpoch;
    double learningRate1;
    double learningRate2;
    int neuronNumber;
    double initTheta1;
    double initTheta2;
};

#endif