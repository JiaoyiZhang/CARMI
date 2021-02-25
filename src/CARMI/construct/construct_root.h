#ifndef CONSTRUCT_ROOT_H
#define CONSTRUCT_ROOT_H

#include "../carmi.h"
#include "store_node.h"
#include "dp.h"

template <typename TYPE>
inline void CARMI::ConstructRoot(TYPE *root, const int childNum, double &totalCost, double &totalTime, double &totalSpace)
{
    vector<pair<int, int>> subInitData(childNum, {-1, 0});
    vector<pair<int, int>> subFindData(childNum, {-1, 0});
    vector<pair<int, int>> subInsertData(childNum, {-1, 0});
    for (int i = 0; i < initDataset.size(); i++)
    {
        int p = root->model.Predict(initDataset[i].first);
        if (subInitData[p].first == -1)
            subInitData[p].first = i;
        subInitData[p].second++;
    }
    for (int i = 0; i < findQuery.size(); i++)
    {
        int p = root->model.Predict(findQuery[i].first);
        if (subFindData[p].first == -1)
            subFindData[p].first = i;
        subFindData[p].second++;
    }
    for (int i = 0; i < insertQuery.size(); i++)
    {
        int p = root->model.Predict(insertQuery[i].first);
        if (subInsertData[p].first == -1)
            subInsertData[p].first = i;
        subInsertData[p].second++;
    }

    for (int i = 0; i < childNum; i++)
    {
        NodeCost resChild;
        if (subInitData[i].second + subInsertData[i].second > kAlgorithmThreshold)
            resChild = GreedyAlgorithm(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
        else
            resChild = dp(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
        int type;
        pair<bool, pair<int, int>> key = {resChild.isInnerNode, {subInitData[i].first, subInitData[i].second}};
        auto it = structMap.find(key);
        if (it == structMap.end())
        {
            if (kPrimaryIndex)
                type = 6;
            else
                type = 4;
        }
        else
            type = it->second.type;

        storeOptimalNode(type, key, subInitData[i].first, subInitData[i].second, subInsertData[i].first, subInsertData[i].second, i);

        totalCost += resChild.space + resChild.time;
        totalTime += resChild.time;
        totalSpace += resChild.space;

        COST.clear();
        structMap.clear();
    }
#ifdef DEBUG
    cout << "construct over!" << endl;
    double entropy = 0.0;
    int size = initDataset.size();
    float p = 0;
    for (int i = 0; i < childNum; i++)
    {
        p = float(subInitData[i].second) / size;
        if (p != 0)
            entropy -= p * log(p) / log(2);
    }
    cout << "entropy: " << entropy;

#endif // DEBUG
}

#endif // !CONSTRUCT_ROOT_H