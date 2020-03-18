#ifndef UPPER_MODEL_H
#define UPPER_MODEL_H

#include "nn.h"
#include "../cpp-btree/btree_map.h"
#include "params.h"
#include <array>

template <typename lowerType, typename mlType>
class upperModel
{
public:
    upperModel(){};
    upperModel(const vector<pair<double, double>> &dataset, params firstStageParams, params secondStageParams, int threshold, int secondStageSize, int maxInsertNumber)
    {
        m_dataset = dataset;
        std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });
        
        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
        m_secondStageSize = secondStageSize;
        // m_maxInsertNumber = maxInsertNumber;

        for (int i = 0; i < m_secondStageSize; i++)
        {
            m_secondStage.push_back(type(threshold, m_secondStageParams, maxInsertNumber));
        }
    }

    void train();

    pair<double, double> find(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx].find(key);
    }
    bool insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx].insert(data);
    }
    bool del(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx].del(key);
    }
    bool update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx].update(data);
    }

private:
    vector<pair<double, double>> m_dataset; // the initial dataset, useless after training
    mlType m_firstStageNetwork = mlType();        // network of the first stage
    // int m_maxInsertNumber;                  // maximum number of inserts

    vector<lowerType> m_secondStage; // store the lower nodes
    params m_firstStageParams; // parameters of network
    params m_secondStageParams; // parameters of lower nodes
    int m_secondStageSize;      // the size of the lower nodes
};

template <typename lowerType, typename mlType>
void upperModel<lowerType, mlType>::train()
{
    cout << "train first stage" << endl;
    m_firstStageNetwork.train(m_dataset, m_firstStageParams);
    array<vector<pair<double, double>>, 128> perSubDataset;
    for (int i = 0; i < m_dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(m_dataset[i].first);
        p = p * (m_secondStageSize - 1); //calculate the index of second stage model
        int preIdx = static_cast<int>(p);
        perSubDataset[preIdx].push_back(m_dataset[i]);
    }
    for (int i = 0; i < m_secondStageSize; i++)
    {
        if (perSubDataset[i].size() >= m_dataset.size() / 2)
            return train();
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < m_secondStageSize; i++)
    {
        m_secondStage[i].train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

#endif