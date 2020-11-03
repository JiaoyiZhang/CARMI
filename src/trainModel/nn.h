#ifndef NN_H
#define NN_H

#include "model.h"
#include "../params.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
using namespace std;

extern const int kMaxEpoch;
extern const double kLearningRate;
extern int kNeuronNumber;

// (1, k) * (k, 1)
inline double mul(vector<double> a, vector<double> b)
{
	double res = 0;
	for (int i = 0; i < a.size(); i++)
	{
		res += a[i] * b[i];
	}
	return res;
}

// (1, 1) * (1, k)
inline vector<double> multiply(double num, vector<double> a)
{
	vector<double> res;
	for (int i = 0; i < a.size(); i++)
	{
		res.push_back(num * a[i]);
	}
	return res;
}

// add (1, k) and b
inline vector<double> add(vector<double> a, double bias)
{
	for (int i = 0; i < a.size(); i++)
	{
		a[i] += bias;
	}
	return a;
}

// add (1, k) and b(1, k)
inline vector<double> add(vector<double> a, vector<double> bias)
{
	for (int i = 0; i < a.size(); i++)
	{
		a[i] += bias[i];
	}
	return a;
}

inline vector<double> Relu(vector<double> input)
{
	for (int i = 0; i < input.size(); i++)
	{
		input[i] = max(double(0), input[i]);
	}
	return input;
}

class parameter
{
public:
	parameter(double p, double w1, double bias, double w2)
	{
		point = p;
		W1 = w1;
		W2 = w2;
		b = bias;
	}

	double point;
	double W1;
	double b;
	double W2;
};
class Net : public BasicModel
{
public:
	Net(){};

	void Train(const vector<pair<double, double>> &dataset, int len);

	int Predict(double key)  // return the index in children
	{
		double p = b2;
		for (int i = 0; i < PositiveSegment.size(); i++)
		{
			if(key > PositiveSegment[i].point)
			{
				p += (key * PositiveSegment[i].W1 + PositiveSegment[i].b) * PositiveSegment[i].W2;
			}
		}
		for (int i = 0; i < NegativeSegment.size(); i++)
		{
			if(key < NegativeSegment[i].point)
			{
				p += (key * NegativeSegment[i].W1 + NegativeSegment[i].b) * NegativeSegment[i].W2;
			}
		}
		if (p < 0)
			p = 0;
		else if (p > length)
			p = length;
		return int(p);
	}

private:
	int length;
	vector<parameter> PositiveSegment;
	vector<parameter> NegativeSegment;
	double b2;
};

// train the Network
void Net::Train(const vector<pair<double, double>> &dataset, int len)
{
	length = len - 1;
	// initialize the parameters
	std::default_random_engine gen;
	std::normal_distribution<double> dis(1, 3);
	vector<double> W1, W2, b1;
	for (int i = 0; i < kNeuronNumber; i++)
	{
		W1.push_back(dis(gen));
		W2.push_back(dis(gen));
		b1.push_back(1);
	}
	b2 = 0;
	vector<pair<double, double>> m_dataset = dataset;
	vector<double> index;
	int datasetSize = m_dataset.size();
	if (datasetSize == 0 || datasetSize < 20)
		return;
	for (int i = 0; i < m_dataset.size(); i++)
	{
		if (m_dataset[i].first != -1)
			index.push_back(double(i) / double(datasetSize));
	}

	double totalLoss = 0.0;
	for (int epoch = 0; epoch < kMaxEpoch; epoch++)
	{
		clock_t s, f;
		s = clock();
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
				else if (p > 1)
					p = 1;

				// calculate the loss
				totalLoss += 0.5 * (p - double(y)) * (p - double(y));

				// backward propogation
				// updata W1 and b1
				for (int j = 0; j < W1.size(); j++)
				{
					if (tempFLR[j] > 0)
					{
						W1[j] = W1[j] - kLearningRate * x * W2[j] * (p - y);
						b1[j] = b1[j] - kLearningRate * W2[j] * (p - y);
					}
				}
				// update W2 and b2
				W2 = add(W2, (multiply(-kLearningRate, multiply(p - y, firstLayerResult)))); // W2 = W2 - lr * firstLayerResult * (p - y)
				b2 = b2 - kLearningRate * (p - y);
			}
		}
		f = clock();
		// cout<<"epoch "<<epoch<<": time: "<<(float)(f-s) / CLOCKS_PER_SEC<<endl;
	}
	// record the split points
	for(int i=0;i<W1.size();i++)
	{
		if(W1[i]>0)
			PositiveSegment.push_back(parameter(- (b1[i] / W1[i]), W1[i], b1[i], W2[i]*len));
		else
			NegativeSegment.push_back(parameter(- (b1[i] / W1[i]), W1[i], b1[i], W2[i]*len));
	}
	b2 *= len;
}
#endif