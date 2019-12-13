// #ifndef NN_H
// #define NN_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
using namespace std;

class net
{
public:
    net(vector<pair<int, int>> dataset, int maxEpochNumber, float lr, int neuronNumber)
    {
        m_dataset = dataset;
        // m_batchSize = batchSize;
        m_learningRate = lr;
        m_maxEpochNumber = maxEpochNumber;
        m_neuronNumber = neuronNumber;
        for (int i = 0; i < dataset.size(); i++)
        {
            index.push_back(i);
        }
    }

    void train();

    float predict(int key); // return the key's index

    vector<float> Relu(vector<float> input);

private:
    vector<pair<int, int>> m_dataset;
    vector<int> index;
    vector<float> W1;
    vector<float> W2;
    vector<float> b1;
    float b2;
    // int m_batchSize;
    int m_maxEpochNumber;
    float m_learningRate;
    int m_neuronNumber;
};

// train the network
void net::train()
{
    int datasetSize = m_dataset.size();
    // initialize the parameters
    for (int i = 0; i < m_neuronNumber; i++)
    {
        W1.push_back(0);
        W2.push_back(0);
        b1.push_back(0);
    }
    b2 = 0;

    cout << "Start training the first layer" << endl;

    for (int epoch = 0; epoch < m_maxEpochNumber; epoch++)
    {
        cout << "epoch: " << epoch;

        unsigned seed = chrono::system_clock::now().time_since_epoch().count();
        shuffle(m_dataset.begin(), m_dataset.end(), default_random_engine(seed));
        shuffle(index.begin(), index.end(), default_random_engine(seed));

        float totalLoss = 0;
        float loss = 0;

        for (int i = 0; i < datasetSize; i++)
        {
            // forward
            int y = index[i];                    // the actual index
            float x = float(m_dataset[i].first); // key
            vector<float> tempFLR = add(multiply(x, W1), b1);
            vector<float> firstLayerResult = Relu(tempFLR); // the result of first layer
            float tempY = mul(firstLayerResult, W2) + b2;
            float p = max(float(0), tempY); // the result of the nn

            // calculate the loss
            loss = 0.5 * (p - float(y)) * (p - float(y));
            totalLoss += loss;

            // backward propogation
            // update W2 and b2, if tempY <= 0, W2 = W2, b2 = b2
            if (tempY > 0)
            {
                W2 = add(W2, (multiply(-m_learningRate, firstLayerResult)));
                b2 = b2 - 1;
            }

            // updata W1 and b1
            for (int j = 0; j < tempFLR.size(); j++)
            {
                if (tempY > 0 && tempFLR[i])
                {
                    W1[i] = W1[i] - m_learningRate * x;
                    b1[i] = b1[i] - 1;
                }
            }

            // print the bp result
            cout << "now W2 is:" << endl;
            for (int m = 0; m < W2.size(); m++)
            {
                cout << W2[m] << " ";
                if (m + 1 % 10 == 0)
                    cout << endl;
            }
            cout << endl;
            cout << "now W1 is:" << endl;
            for (int m = 0; m < W1.size(); m++)
            {
                cout << W1[m] << " ";
                if (m + 1 % 10 == 0)
                    cout << endl;
            }
            cout << endl;
            cout << "now b2 is:" << b2 << endl;
            cout << "now b1 is:" << endl;
            for (int m = 0; m < b1.size(); m++)
            {
                cout << b1[m] << " ";
                if (m + 1 % 10 == 0)
                    cout << endl;
            }
            cout << endl;
        }
        cout << " " << totalLoss << endl;
    }
}

float net::predict(int key)
{
    float x = float(key); // key
    vector<float> firstLayerResult = Relu(add(multiply(x, W1), b1)); // the result of first layer
    float p = max(float(0), mul(firstLayerResult, W2) + b2); // the result of the nn
    return p;
}

vector<float> net::Relu(vector<float> input)
{
    for (int i = 0; i < input.size(); i++)
    {
        input[i] = max(float(0), input[i]);
    }
    return input;
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
vector<float> multiply(float num, vector<float> a)
{
    vector<float> res;
    for (int i = 0; i < a.size(); i++)
    {
        res.push_back(num * a[i]);
    }
    return res;
}

// add (1, k) and b
vector<float> add(vector<float> a, float bias)
{
    for (int i = 0; i < a.size(); i++)
    {
        a[i] += bias;
    }
    return a;
}

// add (1, k) and b(1, k)
vector<float> add(vector<float> a, vector<float> bias)
{
    for (int i = 0; i < a.size(); i++)
    {
        a[i] += bias[i];
    }
    return a;
}

// #endif