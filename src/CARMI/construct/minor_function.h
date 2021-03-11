#ifndef MINOR_FUNCTION_H
#define MINOR_FUNCTION_H

#include "../carmi.h"

double CARMI::CalculateEntropy(int size, int childNum, const vector<DataRange> &perSize) const
{
    double entropy = 0.0;
    for (int i = 0; i < childNum; i++)
    {
        auto p = float(perSize[i].size) / size;
        if (p != 0)
            entropy += p * (-log(p) / log(2));
    }
    return entropy;
}

template <typename TYPE>
void CARMI::NodePartition(const TYPE &node, const DataRange &range, const vector<pair<double, double>> &dataset, vector<DataRange> *subData) const
{
    int end = range.left + range.size;
    for (int i = range.left; i < end; i++)
    {
        int p = node.Predict(dataset[i].first);
        if ((*subData)[p].left == -1)
            (*subData)[p].left = i;
        (*subData)[p].size++;
    }
}

template <typename TYPE>
void CARMI::InnerDivideSingle(int c, const DataRange &range, vector<DataRange> &subDataset)
{
    TYPE node = TYPE();
    node.SetChildNumber(c);
    Train(&node, range.left, range.size);

    NodePartition<TYPE>(node, subDataset, range, initDataset);
}

template <typename TYPE>
TYPE CARMI::InnerDivideAll(int c, const IndexPair &range, SubDataset *subDataset)
{
    TYPE node = TYPE();
    node.SetChildNumber(c);
    Train(&node, range.initRange.left, range.initRange.size);

    NodePartition<TYPE>(node, range.initRange, initDataset, &(subDataset->subInit));
    subDataset->subFind = subDataset->subInit;
    NodePartition<TYPE>(node, range.initRange, initDataset, &(subDataset->subInit));
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