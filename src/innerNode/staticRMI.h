#ifndef STATIC_RMI_H
#define STATIC_RMI_H

#include "../params.h"
#include <array>
#include "../node.h"
#include "../leafNode/gappedNode.h"

template <typename lowerType, typename mlType>
class staticRMI : public node
{
public:
    staticRMI(){};
    staticRMI(const vector<pair<double, double>> &dataset, params firstStageParams, params secondStageParams, int threshold, int secondStageSize, int maxInsertNumber)
    {
        m_dataset = dataset;
        std::sort(m_dataset.begin(), m_dataset.end(), [](pair<double, double> p1, pair<double, double> p2) {
            return p1.first < p2.first;
        });

        m_firstStageParams = firstStageParams;
        m_secondStageParams = secondStageParams;
        m_secondStageSize = secondStageSize;

        for (int i = 0; i < m_secondStageSize; i++)
        {
            m_secondStage.push_back(new lowerType(threshold, m_secondStageParams, maxInsertNumber));
        }
    }

    void train();

    pair<double, double> find(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx]->find(key);
    }
    bool insert(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx]->insert(data);
    }
    bool del(double key)
    {
        double p = m_firstStageNetwork.predict(key);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx]->del(key);
    }
    bool update(pair<double, double> data)
    {
        double p = m_firstStageNetwork.predict(data.first);
        int preIdx = static_cast<int>(p * (m_secondStageSize - 1));
        return m_secondStage[preIdx]->update(data);
    }

    void change(const vector<pair<int, int>> &cnt, int threshold, params secondStageParams, int cap);

private:
    vector<pair<double, double>> m_dataset; // the initial dataset, useless after training
    mlType m_firstStageNetwork = mlType();  // network of the first stage

    vector<node *> m_secondStage; // store the lower nodes
    params m_firstStageParams;    // parameters of network
    params m_secondStageParams;   // parameters of lower nodes
    int m_secondStageSize;        // the size of the lower nodes
};

template <typename lowerType, typename mlType>
void staticRMI<lowerType, mlType>::train()
{
    cout << "train first stage" << endl;
    m_firstStageNetwork.train(m_dataset, m_firstStageParams);
    array<vector<pair<double, double>>, 128> perSubDataset;
    for (int i = 0; i < m_dataset.size(); i++)
    {
        double p = m_firstStageNetwork.predict(m_dataset[i].first);
        // cout << i << " :" << m_dataset[i].first << " p:" << p << "  preIdx: ";
        p = p * (m_secondStageSize - 1);
        int preIdx = static_cast<int>(p);
        // cout << preIdx << endl;
        perSubDataset[preIdx].push_back(m_dataset[i]);
    }
    for (int i = 0; i < m_secondStageSize; i++)
    {
        if (perSubDataset[i].size() == m_dataset.size())
            return train();
    }

    cout << "train second stage" << endl;
    for (int i = 0; i < m_secondStageSize; i++)
    {
        // cout << "second stage " << i << "    datasetSize is:" << perSubDataset[i].size() << endl;
        m_secondStage[i]->train(perSubDataset[i]);
    }
    cout << "End train" << endl;
}

template <typename lowerType, typename mlType>
void staticRMI<lowerType, mlType>::change(const vector<pair<int, int>> &cnt, int threshold, params secondStageParams, int cap)
{
    int idx = 0;
    for (int i = 0; i < m_secondStageSize; i++)
    {
        int r = 0, w = 0;
        vector<pair<double, double>> tmp;
        m_secondStage[i]->getDataset(tmp);
        vector<pair<double, double>> data;
        for (int j = 0; j < tmp.size(); j++)
        {
            if (tmp[j].first != -1)
            {
                r += cnt[idx].first;
                w += cnt[idx].second;
                data.push_back(tmp[j]);
                idx++;
            }
        }
        if ((float)r / (float)w <= 5.0)
        {
            // change from array to gapped array
            gappedNode<mlType> *newNode = new gappedNode<mlType>(threshold, secondStageParams, cap);
            newNode->train(data);
            m_secondStage[i] = newNode;
        }
    }
}
#endif