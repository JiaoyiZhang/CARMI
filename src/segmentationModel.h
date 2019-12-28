#ifndef SEGMENTATION_MODEL_H
#define SEGMENTATION_MODEL_H

#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
class secondStage
{
public:
    secondStage()
    {
        m_datasetSize = 0;
        theta1 = 0.233;
        theta2 = 0.666;
        maxNegativeError = 0;
        maxPositiveError = 0;
    }

    double predict(double x)
    {
        return theta1 * x + theta2;
    }

    void train();

    int getNegativeError()
    {
        return maxNegativeError;
    }

    int getPositiveError()
    {
        return maxPositiveError;
    }

    double getTheta1()
    {
        return theta1;
    }

    double getTheta2()
    {
        return theta2;
    }

    void insert(pair<double, double> data)
    {
        m_dataset.push_back(data);
        m_datasetSize++;
    }

private:
    vector<pair<double, double>> m_dataset;
    int m_datasetSize;
    double theta1;
    double theta2;
    int maxNegativeError;
    int maxPositiveError;
};

void secondStage::train()
{
    if (m_datasetSize == 0)
    {
        cout << "This stage is empty!" << endl;
        return;
    }
    for (int i = 0; i < 5000; i++)
    {
        double error1 = 0.0;
        double error2 = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = predict(m_dataset[j].first);
            p = (p > m_dataset[m_datasetSize].second ? m_dataset[m_datasetSize].second : p);
            p = (p < m_dataset[0].second ? m_dataset[0].second : p);
            double y = m_dataset[j].second;
            error2 += p - y;
            error1 += (p - y) * m_dataset[j].first;
        }
        theta1 = theta1 - 0.000001 * error1 / m_datasetSize;
        theta2 = theta2 - 0.000001 * error2 / m_datasetSize;

        double loss = 0.0;
        for (int j = 0; j < m_datasetSize; j++)
        {
            double p = predict(m_dataset[j].first);
            p = (p > m_dataset[m_datasetSize].second ? m_dataset[m_datasetSize].second : p);
            p = (p < m_dataset[0].second ? m_dataset[0].second : p);
            double y = m_dataset[j].second;
            loss += (p - y) * (p - y);
        }
        loss = loss / (m_datasetSize * 2);
        // cout << "iteration " << i << "    loss is:" << loss << endl;
    }

    for (int i = 0; i < m_datasetSize; i++)
    {
        double p = predict(m_dataset[i].first);
        p = (p > m_dataset[m_datasetSize].second ? m_dataset[m_datasetSize].second : p);
        p = (p < m_dataset[0].second ? m_dataset[0].second : p);
        int y = int(m_dataset[i].second);
        int error = y - int(p);
        if (error > maxPositiveError)
            maxPositiveError = error;
        if (error < maxNegativeError)
            maxNegativeError = error;
    }
}

class segModel
{
public:
    segModel(const vector<pair<double, double>> &dataset)
    {
        m_dataset = dataset;
    }
    void sortData()
    {
        sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
    }

    void preProcess()
    {
        for (int i = 0; i < 100; i++)
        {
            secondStage second = secondStage();
            m_model.push_back(second);
        }
        for (int i = 0; i < m_dataset.size(); i++)
        {
            m_model[int(m_dataset[i].first) % 100].insert({m_dataset[i].first, i});
        }
        for (int i = 0; i < 100; i++)
        {
            m_model[i].train();
            // cout << m_model[i].getNegativeError() << "    " << m_model[i].getPositiveError() << endl;
        }
    }

    pair<double, double> find(double key)
    {
        int secondIndex = int(key) % 100;
        double theta1 = m_model[secondIndex].getTheta1();
        double theta2 = m_model[secondIndex].getTheta2();
        int p = int(theta1 * key + theta2);
        int start = max(0, p + m_model[secondIndex].getNegativeError());
        int end = min(int(m_dataset.size() - 1), p + m_model[secondIndex].getPositiveError());

        int res = -1;
        while (start < end)
        {
            int mid = (start + end) / 2;
            if (m_dataset[mid].first == key)
            {
                res = mid;
                break;
            }
            else if (m_dataset[mid].first > key)
                end = mid;
            else
                start = mid + 1;
        }

        if (res != -1)
            return {key, m_dataset[res].second};
        else
            return {};
    }

private:
    vector<pair<double, double>> m_dataset;
    vector<secondStage> m_model;
};

#endif