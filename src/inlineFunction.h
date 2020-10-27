#ifndef INLINE_FUNCTION_H
#define INLINE_FUNCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include <vector>
using namespace std;

extern vector<void *> INDEX;  // store the entire INDEX
// if the index is adaptive and the next node is 
// array, then check whether we need to 
// convert the current leaf node to an inner node.
// return the type of INDEX[idx]
inline int CheckArray(int idx)
{
    auto leaf = (ArrayType *)INDEX[idx];
    int size = leaf->m_datasetSize;
    if(size >= kMaxKeyNum)
    {
        switch (kInnerNodeID)
        {
        case 0:
        {
            auto newNode = new LRType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new ArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x40000000 + tmpIdx);
                ((ArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;

            // type = kInnerNodeID;
            // currentNode = INDEX[idx];
            return 0;
        }
        break;
        case 1:
        {
            auto newNode = new NNType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new ArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x40000000 + tmpIdx);
                ((ArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            perSubDataset.clear();
            vector<vector<pair<double, double>>>().swap(perSubDataset);

            return 1;
        }
        break;
        case 2:
        {
            auto newNode = new HisType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new ArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x40000000 + tmpIdx);
                ((ArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            perSubDataset.clear();
            vector<vector<pair<double, double>>>().swap(perSubDataset);
            return 2;
        }
        break;
        case 3:
        {
            auto newNode = new BSType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new ArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x40000000 + tmpIdx);
                ((ArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            return 3;
        }
        break;
        }
    }
    return 4;
}

// if the index is adaptive and the next node is 
// gapped array, then check whether we need to 
// convert the current leaf node to an inner node
// return the type of INDEX[idx]
inline int CheckGappedArray(int idx)
{
    auto leaf = (GappedArrayType *)INDEX[idx];
    int size = leaf->m_datasetSize;
    if(size >= kMaxKeyNum)
    {
        switch (kInnerNodeID)
        {
        case 0:
        {
            auto newNode = new LRType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new GappedArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x50000000 + tmpIdx);
                ((GappedArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            // type = kInnerNodeID;
            // currentNode = INDEX[idx];
            return 0;
        }
        break;
        case 1:
        {
            auto newNode = new NNType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new GappedArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x50000000 + tmpIdx);
                ((GappedArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            return 1;
        }
        break;
        case 2:
        {
            auto newNode = new HisType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new GappedArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x50000000 + tmpIdx);
                ((GappedArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            return 2;
        }
        break;
        case 3:
        {
            auto newNode = new BSType(kAdaptiveChildNum);
            // INDEX.push_back((void *)new LRType(childNum));
            vector<pair<double, double>> leafDataset = leaf->m_dataset;
            newNode->model.Train(leafDataset);

            vector<vector<pair<double, double>>> perSubDataset;
            vector<pair<double, double>> temp;
            for (int i = 0; i < kAdaptiveChildNum; i++)
                perSubDataset.push_back(temp);
            for (int i = 0; i < leafDataset.size(); i++)
            {
                double pre = newNode->model.Predict(leafDataset[i].first);
                int pIdx = static_cast<int>(pre*(kAdaptiveChildNum - 1));
                perSubDataset[pIdx].push_back(leafDataset[i]);
            }
            // a number of children leaf level model are created
            for (int i = 0; i < kAdaptiveChildNum; i++)
            {
                INDEX.push_back((void *)new GappedArrayType(kThreshold));
                int tmpIdx = INDEX.size()-1;
                newNode->child.push_back(0x50000000 + tmpIdx);
                ((GappedArrayType*)(INDEX[tmpIdx]))->SetDataset(perSubDataset[i]);
            }
            INDEX[idx] = (void *)newNode;
            return 3;
        }
        break;
        }
    }
    return 5;
}

// search a key-value through binary search in 
// the array leaf node
inline int ArrayBinarySearch(vector<pair<double, double>> &m_dataset, double key, int idx, int start, int end)
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (m_dataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

// search a key-value through binary search 
// in the gapped array
inline int GABinarySearch(vector<pair<double, double>> &m_dataset, double key, int idx, int start_idx, int end_idx)
{
    // use binary search to find
    while (start_idx < end_idx)
    {
        if (m_dataset[start_idx].first == -1)
            start_idx--;
        if (m_dataset[end_idx].first == -1)
            end_idx++;
        int mid = (start_idx + end_idx) >> 1;
        if (m_dataset[mid].first == -1)
        {
            int left = max(start_idx, mid - 1);
            int right = min(end_idx, mid + 1);
            if (m_dataset[left].first >= key)
                end_idx = left;
            else
                start_idx = right;
        }
        else
        {
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    return start_idx;
}

#endif