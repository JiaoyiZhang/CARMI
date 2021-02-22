#ifndef CONSTRUCT_ROOT_H
#define CONSTRCT_ROOT_H

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
        int p = root.model.Predict(initDataset[i].first);
        if (subInitData[p].first == -1)
            subInitData[p].first = i;
        subInitData[p].second++;
    }
    for (int i = 0; i < findQuery.size(); i++)
    {
        int p = root.model.Predict(findQuery[i].first);
        if (subFindData[p].first == -1)
            subFindData[p].first = i;
        subFindData[p].second++;
    }
    for (int i = 0; i < insertQuery.size(); i++)
    {
        int p = root.model.Predict(insertQuery[i].first);
        if (subInsertData[p].first == -1)
            subInsertData[p].first = i;
        subInsertData[p].second++;
    }

    for (int i = 0; i < childNum; i++)
    {
        NodeCost resChild;
        if (subInitData[i].second + subInsertData[i].second > 4096)
            resChild = GreedyAlgorithm(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second); // construct an inner node
        else
            resChild = dp(subInitData[i].first, subInitData[i].second, subFindData[i].first, subFindData[i].second, subInsertData[i].first, subInsertData[i].second);
        int type;
        pair<bool, pair<int, int>> key = {resChild.second, {subInitData[i].first, subInitData[i].second}};
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

        totalCost += resChild.first.first + resChild.first.second;
        totalTime += resChild.first.first;
        totalSpace += resChild.first.second;

        COST.clear();
        structMap.clear();
    }
}

#endif // !CONSTRUCT_ROOT_H