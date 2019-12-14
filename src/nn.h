#ifndef NN_H
#define NN_H

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
using namespace std;

// (1, k) * (k, 1)
double mul(vector<double> a, vector<double> b)
{
    double res = 0;
    for (int i = 0; i < a.size(); i++)
    {
        res += a[i] * b[i];
    }
    return res;
}

// (1, 1) * (1, k)
vector<double> multiply(double num, vector<double> a)
{
    vector<double> res;
    for (int i = 0; i < a.size(); i++)
    {
        res.push_back(num * a[i]);
    }
    return res;
}

// add (1, k) and b
vector<double> add(vector<double> a, double bias)
{
    for (int i = 0; i < a.size(); i++)
    {
        a[i] += bias;
    }
    return a;
}

// add (1, k) and b(1, k)
vector<double> add(vector<double> a, vector<double> bias)
{
    for (int i = 0; i < a.size(); i++)
    {
        a[i] += bias[i];
    }
    return a;
}

class net
{
public:
    net(){};

    void insert(vector<pair<double, double>> dataset, int maxEpochNumber, double lr, int neuronNumber)
    {
        m_dataset = dataset;
		datasetSize = m_dataset.size();
		for (int i = 0; i < datasetSize; i++)
		{
			index.push_back(double(i) / double(datasetSize));
		}
        m_learningRate = lr;
		m_maxEpochNumber = maxEpochNumber;
		m_neuronNumber = neuronNumber;
    }

	void train();

	double predict(int key); // return the key's index

	vector<double> Relu(vector<double> input);

private:
	vector<pair<double, double>> m_dataset;
	vector<double> index;
	vector<double> W1;
	vector<double> W2;
	vector<double> b1;
	double b2;
	int m_maxEpochNumber;
	double m_learningRate;
	int m_neuronNumber;
	int datasetSize;
};

// train the network
void net::train()
{
	// initialize the parameters
	std::default_random_engine gen;
	std::normal_distribution<double> dis(1, 3);
	for (int i = 0; i < m_neuronNumber; i++)
	{
		W1.push_back(dis(gen));
		W2.push_back(dis(gen));
		b1.push_back(1);
	}
	b2 = 0.91;

	cout << "Start training" << endl;
	double totalLoss = 0.0;
	for (int epoch = 0; epoch < m_maxEpochNumber; epoch++)
	{
		// cout << "epoch: " << epoch << endl;

		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		shuffle(m_dataset.begin(), m_dataset.end(), default_random_engine(seed));
		shuffle(index.begin(), index.end(), default_random_engine(seed));
		totalLoss = 0.0;
		double loss = 0.0;

		for (int i = 0; i < datasetSize; i++)
		{
			// forward
			double y = index[i];				   // the actual index
			double x = double(m_dataset[i].first); // key
			vector<double> tempFLR = add(multiply(x, W1), b1);
			vector<double> firstLayerResult = Relu(tempFLR); // the result of first layer
			double p = mul(firstLayerResult, W2) + b2;		 // the result of the nn

			if (p < 0)
				p = 0;
			if (p > 1)
				p = 1;

			// calculate the loss
			loss = 0.5 * (p - double(y)) * (p - double(y));
			totalLoss += loss;

			// backward propogation
			// updata W1 and b1
			for (int j = 0; j < W1.size(); j++)
			{
				if (tempFLR[j] > 0)
				{
					W1[j] = W1[j] - m_learningRate * x * W2[j] * (p - y);
					b1[j] = b1[j] - m_learningRate * W2[j] * (p - y);
				}
			}
			// update W2 and b2
			W2 = add(W2, (multiply(-m_learningRate, multiply(p - y, firstLayerResult)))); // W2 = W2 - lr * firstLayerResult * (p - y)
			b2 = b2 - m_learningRate * (p - y);
		}
		//cout << "    loss is: " << setiosflags(ios::fixed) << setprecision(4) << totalLoss << endl;
	}
	if (totalLoss > 100)
		train();
	else
	{
		cout << "final loss is: " << totalLoss << endl;
	}
}

double net::predict(int key)
{
	double x = double(key);											  // key
	vector<double> firstLayerResult = Relu(add(multiply(x, W1), b1)); // the result of first layer
	double p = mul(firstLayerResult, W2) + b2;						  // the result of the nn
	if (p < 0)
		p = 0;
	if (p > 1)
		p = 1;
	return p;
}

vector<double> net::Relu(vector<double> input)
{
	for (int i = 0; i < input.size(); i++)
	{
		input[i] = max(double(0), input[i]);
	}
	return input;
}

#endif