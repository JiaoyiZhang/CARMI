// #ifndef NN_H
// #define NN_H

#include <iostream>
#include <vector>
using namespace std;

class net
{
public:
    net(vector<int> dataset, int batchSize, int maxEpochNumber, float lr, int neuronNumber)
    {
        m_dataset = dataset;
        m_batchSize = batchSize;
        m_learningRate = lr;
        m_maxEpochNumber = maxEpochNumber;
        m_neuronNumber = neuronNumber;
    }

    void train();

    int predict(int key); // return the key's index

    vector<float> sigmod(vector<float> input);

private:
    vector<int> m_dataset;
    vector<int> W1;
    vector<int> W2;
    float b1;
    float b2;
    int m_batchSize;
    int m_maxEpochNumber;
    float m_learningRate;
    int m_neuronNumber;
};

// train the network
void net::train()
{
    // initialize the parameters
}

vector<float> net::sigmod(vector<float> input)
{
    
}

// (1, k) * (k, 1)
float mul(vector<float> a, vector<float> b)
{
    float res = 0;
    for (int i = 0; i < a.size(); i++)
    {
        res += a[i] * b[i];
    }
    return res;
}

// (1, 1) * (1, k)
vector<float> mul(float num, vector<float> a)
{
    vector<float> res;
    for (int i = 0; i < a.size(); i++)
    {
        res.push_back(num * a[i]);
    }
    return res;
}

// #endif