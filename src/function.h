#ifndef FUNCTION_H
#define FUNCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include "inlineFunction.h"
#include <float.h>
#include <vector>
using namespace std;

vector<LRType> LRVector;
vector<NNType> NNVector;
vector<HisType> HisVector;
vector<BSType> BSVector;
vector<ArrayType> ArrayVector;
vector<GappedArrayType> GAVector;

extern int kLeafNodeID;
extern int kInnerNodeID;

extern int kThreshold;
extern const double kDensity;
extern const int kAdaptiveChildNum;
extern double kRate;
extern const double kReadWriteRate;
extern int kMaxKeyNum;

void Initialize(const vector<pair<double, double> > &dataset, int childNum)
{
    // create the root node
    switch (kInnerNodeID)
    {
    case 0:
        LRVector.push_back(LRType(childNum));
        LRVector[0].Initialize(dataset);
        break;
    case 1:
        NNVector.push_back(NNType(childNum));
        NNVector[0].Initialize(dataset);
        break;
    case 2:
        HisVector.push_back(HisType(childNum));
        HisVector[0].Initialize(dataset);
        break;
    case 3:
        BSVector.push_back(BSType(childNum));
        BSVector[0].Initialize(dataset);
        break;
    }
}

pair<double, double> Find(double key)
{
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            auto node = LRVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            auto node = NNVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            auto node = HisVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;

        }
        break;
        case 3:
        {
            auto node = BSVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;

        }
        break;
        case 4:
        {
            auto leaf = ArrayVector[idx];

            if (leaf.m_datasetSize == 0)
                return {};
            idx = leaf.model.Predict(key);
            if (leaf.m_dataset[idx].first == key)
            {
                return leaf.m_dataset[idx];
            }
            else
            {
                int start = max(0, idx - leaf.error);
                int end = min(leaf.m_datasetSize - 1, idx + leaf.error);
                start = min(start, end);
                int res;
                if(key <= leaf.m_dataset[start].first)
                    res = ArrayBinarySearch(leaf.m_dataset, key, 0, start);
                else if(key <= leaf.m_dataset[end].first)
                    res = ArrayBinarySearch(leaf.m_dataset, key, start, end);
                else
                {
                    res = ArrayBinarySearch(leaf.m_dataset, key, end, leaf.m_datasetSize - 1);
                    if (res >= leaf.m_datasetSize)
                        return {};
                }
                    
                if (leaf.m_dataset[res].first == key)
                    return leaf.m_dataset[res];
                return {};
            }

        }
        break;
        case 5:
        {
            auto galeaf = GAVector[idx];
            idx = galeaf.model.Predict(key);
            if (galeaf.m_dataset[idx].first == key)
                return galeaf.m_dataset[idx];
            else
            {
                int start = max(0, idx - galeaf.error);
                int end = min(galeaf.maxIndex, idx + galeaf.error);
                start = min(start, end);
                
                int res;
                if(key <= galeaf.m_dataset[start].first)
                    res = GABinarySearch(galeaf.m_dataset, key, 0, start);
                else if(key <= galeaf.m_dataset[end].first)
                    res = GABinarySearch(galeaf.m_dataset, key, start, end);
                else
                {
                    res = GABinarySearch(galeaf.m_dataset, key, end, galeaf.maxIndex - 1);
                    if (res >= galeaf.maxIndex)
                        return {DBL_MIN, DBL_MIN};
                }

                if (galeaf.m_dataset[res].first == key)
                    return galeaf.m_dataset[res];
                return {DBL_MIN, DBL_MIN};
            }
        }
        break;
        }
    }
}

bool Insert(pair<double, double> data)
{
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            auto node = LRVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            auto node = NNVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            auto node = HisVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            auto node = BSVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto leaf = ArrayVector[idx];
            
            if (leaf.m_datasetSize == 0)
            {
                leaf.m_dataset.push_back(data);
                leaf.m_datasetSize++;
                leaf.writeTimes++;
                leaf.SetDataset(leaf.m_dataset);
                ArrayVector[idx] = leaf;
                return true;
            }
            int preIdx = leaf.model.Predict(data.first);
            int start = max(0, preIdx - leaf.error);
            int end = min(leaf.m_datasetSize - 1, preIdx + leaf.error);
            start = min(start, end);
            
            if(data.first <= leaf.m_dataset[start].first)
                preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, 0, start);
            else if(data.first <= leaf.m_dataset[end].first)
                preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, start, end);
            else
                preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, end, leaf.m_datasetSize - 1);


            // Insert data
            if (preIdx == leaf.m_datasetSize - 1 && leaf.m_dataset[preIdx].first < data.first)
            {
                leaf.m_dataset.push_back(data);
                leaf.m_datasetSize++;
                leaf.writeTimes++;
                ArrayVector[idx] = leaf;
                return true;
            }
            leaf.m_dataset.push_back(leaf.m_dataset[leaf.m_datasetSize - 1]);
            leaf.m_datasetSize++;
            for (int i = leaf.m_datasetSize - 2; i > preIdx; i--)
                leaf.m_dataset[i] = leaf.m_dataset[i - 1];
            leaf.m_dataset[preIdx] = data;

            leaf.writeTimes++;

            // If the current number is greater than the maximum,
            // the child node needs to be retrained
            if (leaf.writeTimes >= leaf.m_datasetSize || leaf.writeTimes > leaf.m_maxNumber)
                leaf.SetDataset(leaf.m_dataset);
            ArrayVector[idx] = leaf;
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = GAVector[idx];
            if ((float(galeaf.m_datasetSize) / galeaf.capacity > galeaf.density))
                {
                    // If an additional Insertion results in crossing the density
                    // then we expand the gapped array
                    galeaf.SetDataset(galeaf.m_dataset);
                }

                if (galeaf.m_datasetSize == 0)
                {
                    galeaf.m_dataset = vector<pair<double, double>>(galeaf.capacity, pair<double, double>{-1, -1});
                    galeaf.m_dataset[0] = data;
                    galeaf.m_datasetSize++;
                    galeaf.maxIndex = 0;
                    galeaf.SetDataset(galeaf.m_dataset);
                    GAVector[idx] = galeaf;
                    return true;
                }
                int preIdx = galeaf.model.Predict(data.first);

                int start = max(0, preIdx - galeaf.error);
                int end = min(galeaf.maxIndex, preIdx + galeaf.error);
                start = min(start, end);
                
                if(data.first <= galeaf.m_dataset[start].first)
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, 0, start);
                else if(data.first <= galeaf.m_dataset[end].first)
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, start, end);
                else
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, end, galeaf.maxIndex);

                // if the Insertion position is a gap,
                //  then we Insert the element into the gap and are done
                if (galeaf.m_dataset[preIdx].first == -1)
                {
                    galeaf.m_dataset[preIdx] = data;
                    galeaf.m_datasetSize++;
                    galeaf.maxIndex = max(galeaf.maxIndex, preIdx);
                    GAVector[idx] = galeaf;
                    return true;
                }
                else
                {
                    if (galeaf.m_dataset[preIdx].second == DBL_MIN)
                    {
                        galeaf.m_dataset[preIdx] = data;
                        galeaf.m_datasetSize++;
                        galeaf.maxIndex = max(galeaf.maxIndex, preIdx);
                        GAVector[idx] = galeaf;
                        return true;
                    }
                    if (preIdx == galeaf.maxIndex && galeaf.m_dataset[galeaf.maxIndex].first < data.first)
                    {
                        galeaf.m_dataset[++galeaf.maxIndex] = data;
                        galeaf.m_datasetSize++;
                        GAVector[idx] = galeaf;
                        return true;
                    }
                    // If the Insertion position is not a gap, we make
                    // a gap at the Insertion position by shifting the elements
                    // by one position in the direction of the closest gap

                    int i = preIdx + 1;
                    while (galeaf.m_dataset[i].first != -1)
                        i++;
                    if (i >= galeaf.capacity)
                    {
                        i = preIdx - 1;
                        while (i >= 0 && galeaf.m_dataset[i].first != -1)
                            i--;
                        for (int j = i; j < preIdx - 1; j++)
                            galeaf.m_dataset[j] = galeaf.m_dataset[j + 1];
                        preIdx--;
                    }
                    else
                    {
                        if (i > galeaf.maxIndex)
                            galeaf.maxIndex++;
                        for (; i > preIdx; i--)
                            galeaf.m_dataset[i] = galeaf.m_dataset[i - 1];
                    }
                    galeaf.m_dataset[preIdx] = data;
                    galeaf.m_datasetSize++;
                    galeaf.maxIndex = max(galeaf.maxIndex, preIdx);
                    GAVector[idx] = galeaf;
                    return true;
                }
                return false;
        }
        break;
        }
    }
}

bool Delete(double key)
{
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            auto node = LRVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            auto node = NNVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            auto node = HisVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            auto node = BSVector[idx];
            content = node.child[node.model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto leaf = ArrayVector[idx];
            int preIdx = leaf.model.Predict(key);
            if (leaf.m_dataset[preIdx].first != key)
            {
                int start = max(0, preIdx - leaf.error);
                int end = min(leaf.m_datasetSize - 1, preIdx + leaf.error);
                start = min(start, end);
                int res;
                if(key <= leaf.m_dataset[start].first)
                    res = ArrayBinarySearch(leaf.m_dataset, key, 0, start);
                else if(key <= leaf.m_dataset[end].first)
                    res = ArrayBinarySearch(leaf.m_dataset, key, start, end);
                else
                {
                    res = ArrayBinarySearch(leaf.m_dataset, key, end, leaf.m_datasetSize - 1);
                    if (res >= leaf.m_datasetSize)
                        return false;
                }
                if (leaf.m_dataset[res].first == key)
                    preIdx = res;
                else
                    return false;
            }
            for (int i = preIdx; i < leaf.m_datasetSize - 1; i++)
                leaf.m_dataset[i] = leaf.m_dataset[i + 1];
            leaf.m_datasetSize--;
            leaf.m_dataset.pop_back();
            leaf.writeTimes++;
            ArrayVector[idx] = leaf;
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = GAVector[idx];
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            int preIdx = galeaf.model.Predict(key);
            if (galeaf.m_dataset[preIdx].first == key)
            {
                galeaf.m_dataset[preIdx].second = DBL_MIN;
                galeaf.m_datasetSize--;
                if (preIdx == galeaf.maxIndex)
                    galeaf.maxIndex--;
                GAVector[idx] = galeaf;
                return true;
            }
            else
            {
                int start = max(0, preIdx - galeaf.error);
                int end = min(galeaf.maxIndex, preIdx + galeaf.error);
                start = min(start, end);

                int res;
                if(key <= galeaf.m_dataset[start].first)
                    res = GABinarySearch(galeaf.m_dataset, key, 0, start);
                else if(key <= galeaf.m_dataset[end].first)
                    res = GABinarySearch(galeaf.m_dataset, key, start, end);
                else
                {
                    res = GABinarySearch(galeaf.m_dataset, key, end, galeaf.maxIndex);
                    if (res > galeaf.maxIndex)
                        return false;
                }

                if (galeaf.m_dataset[res].first != key)
                    return false;
                galeaf.m_datasetSize--;
                galeaf.m_dataset[res].second = DBL_MIN;
                if (res == galeaf.maxIndex)
                    galeaf.maxIndex--;
                GAVector[idx] = galeaf;
                return true;
            }
        }
        break;
        }
    }
}

bool Update(pair<double, double> data)
{
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            auto node = LRVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            auto node = NNVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            auto node = HisVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            auto node = BSVector[idx];
            content = node.child[node.model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto leaf = ArrayVector[idx];
            int preIdx = leaf.model.Predict(data.first);
            if (leaf.m_dataset[preIdx].first != data.first)
            {
                int start = max(0, preIdx - leaf.error);
                int end = min(leaf.m_datasetSize - 1, preIdx + leaf.error);
                start = min(start, end);
                if(data.first <= leaf.m_dataset[start].first)
                    preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, 0, start);
                else if(data.first <= leaf.m_dataset[end].first)
                    preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, start, end);
                else
                {
                    preIdx = ArrayBinarySearch(leaf.m_dataset, data.first, end, leaf.m_datasetSize - 1);
                    if (preIdx >= leaf.m_datasetSize)
                        return false;

                }
                if (leaf.m_dataset[preIdx].first != data.first)
                    return false;
            }
            leaf.m_dataset[preIdx].second = data.second;
            ArrayVector[idx] = leaf;
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = GAVector[idx];
            int preIdx = galeaf.model.Predict(data.first);
            if (galeaf.m_dataset[preIdx].first == data.first)
            {
                galeaf.m_dataset[preIdx].second = data.second;
                GAVector[idx] = galeaf;
                return true;
            }
            else
            {
                int start = max(0, preIdx - galeaf.error);
                int end = min(galeaf.maxIndex, preIdx + galeaf.error);
                start = min(start, end);

                
                if(data.first <= galeaf.m_dataset[start].first)
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, 0, start);
                else if(data.first <= galeaf.m_dataset[end].first)
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, start, end);
                else
                {
                    preIdx = GABinarySearch(galeaf.m_dataset, data.first, end, galeaf.maxIndex);
                    if (preIdx > galeaf.maxIndex)
                        return false;
                }

                if (galeaf.m_dataset[preIdx].first != data.first)
                    return false;
                galeaf.m_dataset[preIdx].second = data.second;
                GAVector[idx] = galeaf;
                return true;
            }
        }
        break;
        }
    }
}

#endif



