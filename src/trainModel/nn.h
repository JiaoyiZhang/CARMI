#ifndef NN_H
#define NN_H

#include "../params.h"
#include "model.h"
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

class net : public basicModel
{
public:
	net(){};

	void train(const vector<pair<double, double>> &dataset, params param);

	double predict(double key); // return the key's index

	vector<double> Relu(vector<double> input);

private:
	vector<double> W1;
	vector<double> W2;
	vector<double> b1;
	double b2;
};

// train the network
void net::train(const vector<pair<double, double>> &dataset, params param)
{
	vector<pair<double, double>> m_dataset = dataset;
	vector<double> index;
	int datasetSize = m_dataset.size();
	if (datasetSize == 0)
		return;
	for (int i = 0; i < m_dataset.size(); i++)
	{
		if (m_dataset[i].first != -1)
			index.push_back(double(i) / double(datasetSize));
	}

	// initialize the parameters
	std::default_random_engine gen;
	std::normal_distribution<double> dis(1, 3);
	for (int i = 0; i < param.neuronNumber; i++)
	{
		W1.push_back(dis(gen));
		W2.push_back(dis(gen));
		b1.push_back(1);
	}
	b2 = 0.91;

	double totalLoss = 0.0;
	for (int epoch = 0; epoch < param.maxEpoch; epoch++)
	{
		unsigned seed = chrono::system_clock::now().time_since_epoch().count();
		shuffle(m_dataset.begin(), m_dataset.end(), default_random_engine(seed));
		shuffle(index.begin(), index.end(), default_random_engine(seed));
		totalLoss = 0.0;

		for (int i = 0; i < datasetSize; i++)
		{
			if (m_dataset[i].first != -1)
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
				totalLoss += 0.5 * (p - double(y)) * (p - double(y));

				// backward propogation
				// updata W1 and b1
				for (int j = 0; j < W1.size(); j++)
				{
					if (tempFLR[j] > 0)
					{
						W1[j] = W1[j] - param.learningRate1 * x * W2[j] * (p - y);
						b1[j] = b1[j] - param.learningRate2 * W2[j] * (p - y);
					}
				}
				// update W2 and b2
				W2 = add(W2, (multiply(-param.learningRate1, multiply(p - y, firstLayerResult)))); // W2 = W2 - lr * firstLayerResult * (p - y)
				b2 = b2 - param.learningRate2 * (p - y);
			}
		}
	}
	if ((m_dataset.size() / totalLoss) > 60 && totalLoss > 100)
	{
		cout << "RETRAIN" << endl;
		train(dataset, param);
	}
}

double net::predict(double key)
{
	double p = b2;
	// vector<double> firstLayerResult;
	for (int i = 0; i < W1.size(); i++)
	{
		p += max(double(0), key *W1[i] + b1[i]) * W2[i];
	}
	// vector<double> firstLayerResult = Relu(add(multiply(key, W1), b1)); // the result of first layer
	// double p = mul(firstLayerResult, W2) + b2;							// the result of the nn
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