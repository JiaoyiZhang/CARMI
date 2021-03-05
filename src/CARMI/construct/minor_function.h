#ifndef MINOR_FUNCTION_H
#define MINOR_FUNCTION_H

#include "../carmi.h"

double CARMI::CalculateEntropy(const vector<int> &perSize, int size, int childNum) const
{
    double entropy = 0.0;
    for (int i = 0; i < childNum; i++)
    {
        auto p = float(perSize[i]) / size;
        if (p != 0)
            entropy += p * (-log(p) / log(2));
    }
    return entropy;
}

template <typename TYPE>
void CARMI::NodePartition(TYPE *node, SubSingleDataset *subData, SingleDataRange *range, const vector<pair<double, double>> &dataset) const
{
    int end = range->left + range->size;
    for (int i = range->left; i < end; i++)
    {
        int p = node->Predict(dataset[i].first);
        if (subData->subLeft[p] == -1)
            subData->subLeft[p] = i;
        subData->subSize[p]++;
    }
}

template <typename TYPE>
void CARMI::InnerDivideSingle(int c, SingleDataRange *range, SubSingleDataset *subDataset)
{
    TYPE node = TYPE();
    node->SetChildNumber(c);
    Train(&node, range->left, range->size);

    NodePartition<TYPE>(node, subDataset, range, initDataset);
}

template <typename TYPE>
TYPE *CARMI::InnerDivideAll(int c, DataRange *range, SubDataset *subDataset)
{
    TYPE *node = new TYPE();
    node->SetChildNumber(c);
    Train(node, range->initRange.left, range->initRange.size);

    NodePartition<TYPE>(node, subDataset->subInit, range->initRange, initDataset);
    subDataset->subFind = subDataset->subInit;
    NodePartition<TYPE>(node, subDataset->subInsert, range->insertRange, insertQuery);
    return node;
}

void CARMI::UpdateLeaf()
{
    if (kPrimaryIndex)
        return;
    auto it = scanLeaf.begin();
    int pre = it->second;
    auto next = it;
    next++;
    entireChild[it->second].array.nextLeaf = next->second;
    it++;

    for (; it != scanLeaf.end(); it++)
    {
        next = it;
        next++;
        if (next == scanLeaf.end())
        {
            entireChild[it->second].array.previousLeaf = pre;
        }
        else
        {
            entireChild[it->second].array.previousLeaf = pre;
            pre = it->second;
            entireChild[it->second].array.nextLeaf = next->second;
        }
    }
}
#endif // !MINOR_FUNCTION_H