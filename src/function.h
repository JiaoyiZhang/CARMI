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
#include <math.h>
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
extern int kMaxKeyNum;

void Initialize(const vector<pair<double, double>> &dataset, int childNum)
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

pair<double, double> Find(int rootType, double key)
{
    int idx = 0; // idx in the INDEX
    int content;
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            content = LRVector[idx].child[LRVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = NNVector[idx].child[NNVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = HisVector[idx].child[HisVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = BSVector[idx].child[BSVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            if (ArrayVector[idx].m_datasetSize == 0)
                return {};
            int preIdx = ArrayVector[idx].model.Predict(key);
            if (ArrayVector[idx].m_dataset[preIdx].first == key)
            {
                return ArrayVector[idx].m_dataset[preIdx];
            }
            else
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                int res;
                if (key <= ArrayVector[idx].m_dataset[start].first)
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, 0, start);
                else if (key <= ArrayVector[idx].m_dataset[end].first)
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, start, end);
                else
                {
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, end, ArrayVector[idx].m_datasetSize - 1);
                    if (res >= ArrayVector[idx].m_datasetSize)
                        return {};
                }

                if (ArrayVector[idx].m_dataset[res].first == key)
                    return ArrayVector[idx].m_dataset[res];
                return {};
            }
        }
        break;
        case 5:
        {
            int preIdx = GAVector[idx].model.Predict(key);
            if (GAVector[idx].m_dataset[preIdx].first == key)
                return GAVector[idx].m_dataset[preIdx];
            else
            {
                int start = max(0, preIdx - GAVector[idx].error);
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
                start = min(start, end);

                int res;
                if (GAVector[idx].m_dataset[start].first == -1)
                    start--;
                if (GAVector[idx].m_dataset[end].first == -1)
                    end--;
                if (key <= GAVector[idx].m_dataset[start].first)
                    res = GABinarySearch(GAVector[idx].m_dataset, key, 0, start);
                else if (key <= GAVector[idx].m_dataset[end].first)
                    res = GABinarySearch(GAVector[idx].m_dataset, key, start, end);
                else
                {
                    res = GABinarySearch(GAVector[idx].m_dataset, key, end, GAVector[idx].maxIndex - 1);
                    if (res >= GAVector[idx].maxIndex)
                        return {DBL_MIN, DBL_MIN};
                }

                if (GAVector[idx].m_dataset[res].first == key)
                    return GAVector[idx].m_dataset[res];
                return {DBL_MIN, DBL_MIN};
            }
        }
        break;
        }
    }
}

bool Insert(int rootType, pair<double, double> data)
{
    int idx = 0; // idx in the INDEX
    int content;
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            content = LRVector[idx].child[LRVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = NNVector[idx].child[NNVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = HisVector[idx].child[HisVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = BSVector[idx].child[BSVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            if (ArrayVector[idx].m_datasetSize == 0)
            {
                ArrayVector[idx].m_dataset.push_back(data);
                ArrayVector[idx].m_datasetSize++;
                ArrayVector[idx].writeTimes++;
                ArrayVector[idx].SetDataset(ArrayVector[idx].m_dataset);
                return true;
            }
            int preIdx = ArrayVector[idx].model.Predict(data.first);
            int start = max(0, preIdx - ArrayVector[idx].error);
            int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
            start = min(start, end);

            if (data.first <= ArrayVector[idx].m_dataset[start].first)
                preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, 0, start);
            else if (data.first <= ArrayVector[idx].m_dataset[end].first)
                preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, start, end);
            else
                preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, end, ArrayVector[idx].m_datasetSize - 1);

            // Insert data
            if (preIdx == ArrayVector[idx].m_datasetSize - 1 && ArrayVector[idx].m_dataset[preIdx].first < data.first)
            {
                ArrayVector[idx].m_dataset.push_back(data);
                ArrayVector[idx].m_datasetSize++;
                ArrayVector[idx].writeTimes++;
                return true;
            }
            ArrayVector[idx].m_dataset.push_back(ArrayVector[idx].m_dataset[ArrayVector[idx].m_datasetSize - 1]);
            ArrayVector[idx].m_datasetSize++;
            for (int i = ArrayVector[idx].m_datasetSize - 2; i > preIdx; i--)
                ArrayVector[idx].m_dataset[i] = ArrayVector[idx].m_dataset[i - 1];
            ArrayVector[idx].m_dataset[preIdx] = data;

            ArrayVector[idx].writeTimes++;

            // If the current number is greater than the maximum,
            // the child node needs to be retrained
            if (ArrayVector[idx].writeTimes >= ArrayVector[idx].m_datasetSize || ArrayVector[idx].writeTimes > ArrayVector[idx].m_maxNumber)
                ArrayVector[idx].SetDataset(ArrayVector[idx].m_dataset);
            return true;
        }
        break;
        case 5:
        {
            if ((float(GAVector[idx].m_datasetSize) / GAVector[idx].capacity > GAVector[idx].density))
            {
                // If an additional Insertion results in crossing the density
                // then we expand the gapped array
                GAVector[idx].SetDataset(GAVector[idx].m_dataset);
            }

            if (GAVector[idx].m_datasetSize == 0)
            {
                GAVector[idx].m_dataset = vector<pair<double, double>>(GAVector[idx].capacity, pair<double, double>{-1, -1});
                GAVector[idx].m_dataset[0] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = 0;
                GAVector[idx].SetDataset(GAVector[idx].m_dataset);
                return true;
            }
            int preIdx = GAVector[idx].model.Predict(data.first);

            int start = max(0, preIdx - GAVector[idx].error);
            int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
            start = min(start, end);

            if (GAVector[idx].m_dataset[start].first == -1)
                start--;
            if (GAVector[idx].m_dataset[end].first == -1)
                end--;

            if (data.first <= GAVector[idx].m_dataset[start].first)
                preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, 0, start);
            else if (data.first <= GAVector[idx].m_dataset[end].first)
                preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, start, end);
            else
                preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, end, GAVector[idx].maxIndex);

            // if the Insertion position is a gap,
            //  then we Insert the element into the gap and are done
            if (GAVector[idx].m_dataset[preIdx].first == -1)
            {
                GAVector[idx].m_dataset[preIdx] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx);
                return true;
            }
            else
            {
                if (GAVector[idx].m_dataset[preIdx].second == DBL_MIN)
                {
                    GAVector[idx].m_dataset[preIdx] = data;
                    GAVector[idx].m_datasetSize++;
                    GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx);
                    return true;
                }
                if (preIdx == GAVector[idx].maxIndex && GAVector[idx].m_dataset[GAVector[idx].maxIndex].first < data.first)
                {
                    GAVector[idx].m_dataset[++GAVector[idx].maxIndex] = data;
                    GAVector[idx].m_datasetSize++;
                    return true;
                }
                // If the Insertion position is not a gap, we make
                // a gap at the Insertion position by shifting the elements
                // by one position in the direction of the closest gap

                int i = preIdx + 1;
                while (GAVector[idx].m_dataset[i].first != -1)
                    i++;
                if (i >= GAVector[idx].capacity)
                {
                    i = preIdx - 1;
                    while (i >= 0 && GAVector[idx].m_dataset[i].first != -1)
                        i--;
                    for (int j = i; j < preIdx - 1; j++)
                        GAVector[idx].m_dataset[j] = GAVector[idx].m_dataset[j + 1];
                    preIdx--;
                }
                else
                {
                    if (i > GAVector[idx].maxIndex)
                        GAVector[idx].maxIndex++;
                    for (; i > preIdx; i--)
                        GAVector[idx].m_dataset[i] = GAVector[idx].m_dataset[i - 1];
                }
                GAVector[idx].m_dataset[preIdx] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx);
                return true;
            }
            return false;
        }
        break;
        }
    }
}

bool Delete(int rootType, double key)
{
    int idx = 0; // idx in the INDEX
    int content;
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            content = LRVector[idx].child[LRVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = NNVector[idx].child[NNVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = HisVector[idx].child[HisVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = BSVector[idx].child[BSVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            int preIdx = ArrayVector[idx].model.Predict(key);
            if (ArrayVector[idx].m_dataset[preIdx].first != key)
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                int res;
                if (key <= ArrayVector[idx].m_dataset[start].first)
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, 0, start);
                else if (key <= ArrayVector[idx].m_dataset[end].first)
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, start, end);
                else
                {
                    res = ArrayBinarySearch(ArrayVector[idx].m_dataset, key, end, ArrayVector[idx].m_datasetSize - 1);
                    if (res >= ArrayVector[idx].m_datasetSize)
                        return false;
                }
                if (ArrayVector[idx].m_dataset[res].first == key)
                    preIdx = res;
                else
                    return false;
            }
            for (int i = preIdx; i < ArrayVector[idx].m_datasetSize - 1; i++)
                ArrayVector[idx].m_dataset[i] = ArrayVector[idx].m_dataset[i + 1];
            ArrayVector[idx].m_datasetSize--;
            ArrayVector[idx].m_dataset.pop_back();
            ArrayVector[idx].writeTimes++;
            return true;
        }
        break;
        case 5:
        {
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            int preIdx = GAVector[idx].model.Predict(key);
            if (GAVector[idx].m_dataset[preIdx].first == key)
            {
                GAVector[idx].m_dataset[preIdx].second = DBL_MIN;
                GAVector[idx].m_datasetSize--;
                if (preIdx == GAVector[idx].maxIndex)
                    GAVector[idx].maxIndex--;
                return true;
            }
            else
            {
                int start = max(0, preIdx - GAVector[idx].error);
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
                start = min(start, end);

                int res;
                if (GAVector[idx].m_dataset[start].first == -1)
                    start--;
                if (GAVector[idx].m_dataset[end].first == -1)
                    end--;
                if (key <= GAVector[idx].m_dataset[start].first)
                    res = GABinarySearch(GAVector[idx].m_dataset, key, 0, start);
                else if (key <= GAVector[idx].m_dataset[end].first)
                    res = GABinarySearch(GAVector[idx].m_dataset, key, start, end);
                else
                {
                    res = GABinarySearch(GAVector[idx].m_dataset, key, end, GAVector[idx].maxIndex);
                    if (res > GAVector[idx].maxIndex)
                        return false;
                }

                if (GAVector[idx].m_dataset[res].first != key)
                    return false;
                GAVector[idx].m_datasetSize--;
                GAVector[idx].m_dataset[res].second = DBL_MIN;
                if (res == GAVector[idx].maxIndex)
                    GAVector[idx].maxIndex--;
                return true;
            }
        }
        break;
        }
    }
}

bool Update(int rootType, pair<double, double> data)
{
    int idx = 0; // idx in the INDEX
    int content;
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            content = LRVector[idx].child[LRVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = NNVector[idx].child[NNVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = HisVector[idx].child[HisVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = BSVector[idx].child[BSVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            int preIdx = ArrayVector[idx].model.Predict(data.first);
            if (ArrayVector[idx].m_dataset[preIdx].first != data.first)
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                if (data.first <= ArrayVector[idx].m_dataset[start].first)
                    preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, 0, start);
                else if (data.first <= ArrayVector[idx].m_dataset[end].first)
                    preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, start, end);
                else
                {
                    preIdx = ArrayBinarySearch(ArrayVector[idx].m_dataset, data.first, end, ArrayVector[idx].m_datasetSize - 1);
                    if (preIdx >= ArrayVector[idx].m_datasetSize)
                        return false;
                }
                if (ArrayVector[idx].m_dataset[preIdx].first != data.first)
                    return false;
            }
            ArrayVector[idx].m_dataset[preIdx].second = data.second;
            return true;
        }
        break;
        case 5:
        {
            int preIdx = GAVector[idx].model.Predict(data.first);
            if (GAVector[idx].m_dataset[preIdx].first == data.first)
            {
                GAVector[idx].m_dataset[preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - GAVector[idx].error);
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
                start = min(start, end);
                if (GAVector[idx].m_dataset[start].first == -1)
                    start--;
                if (GAVector[idx].m_dataset[end].first == -1)
                    end--;

                if (data.first <= GAVector[idx].m_dataset[start].first)
                    preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, 0, start);
                else if (data.first <= GAVector[idx].m_dataset[end].first)
                    preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, start, end);
                else
                {
                    preIdx = GABinarySearch(GAVector[idx].m_dataset, data.first, end, GAVector[idx].maxIndex);
                    if (preIdx > GAVector[idx].maxIndex)
                        return false;
                }

                if (GAVector[idx].m_dataset[preIdx].first != data.first)
                    return false;
                GAVector[idx].m_dataset[preIdx].second = data.second;
                return true;
            }
        }
        break;
        }
    }
}

long double GetEntropy(int size)
{
    // the entropy of each type of leaf node is the same
    long double entropy = 0.0;
    for (int i = 0; i < ArrayVector.size(); i++)
    {
        if (ArrayVector[i].m_datasetSize != 0)
        {
            auto p = double(ArrayVector[i].m_datasetSize) / size;
            entropy += p * (-log(p) / log(2));
        }
    }
    return entropy;
}

#endif
