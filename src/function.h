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

extern vector<vector<pair<double, double>>> entireDataset;

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
            int preIdx = ArrayVector[idx].model.PredictPrecision(key, ArrayVector[idx].m_datasetSize);
            auto entireIdx = ArrayVector[idx].datasetIndex;
            if (entireDataset[entireIdx][preIdx].first == key)
            {
                return entireDataset[entireIdx][preIdx];
            }
            else
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                int res;
                if (key <= entireDataset[entireIdx][start].first)
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, 0, start);
                else if (key <= entireDataset[entireIdx][end].first)
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, start, end);
                else
                {
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, end, ArrayVector[idx].m_datasetSize - 1);
                    if (res >= ArrayVector[idx].m_datasetSize)
                        return {};
                }
                if (entireDataset[entireIdx][res].first == key)
                    return entireDataset[entireIdx][res];
                return {};
            }
        }
        break;
        case 5:
        {
            int preIdx = GAVector[idx].model.PredictPrecision(key, GAVector[idx].maxIndex + 1);
            auto entireIdx = GAVector[idx].datasetIndex;
            if (entireDataset[entireIdx][preIdx].first == key)
                return entireDataset[entireIdx][preIdx];
            else
            {
                int start = max(0, preIdx - GAVector[idx].error);
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
                start = min(start, end);

                int res;
                if (entireDataset[entireIdx][start].first == -1)
                    start--;
                if (entireDataset[entireIdx][end].first == -1)
                    end--;
                if (key <= entireDataset[entireIdx][start].first)
                    res = GABinarySearch(entireDataset[entireIdx], key, 0, start);
                else if (key <= entireDataset[entireIdx][end].first)
                    res = GABinarySearch(entireDataset[entireIdx], key, start, end);
                else
                {
                    res = GABinarySearch(entireDataset[entireIdx], key, end, GAVector[idx].maxIndex - 1);
                    if (res >= GAVector[idx].maxIndex)
                        return {DBL_MIN, DBL_MIN};
                }

                if (entireDataset[entireIdx][res].first == key)
                    return entireDataset[entireIdx][res];
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
            auto entireIdx = ArrayVector[idx].datasetIndex;
            if (ArrayVector[idx].m_datasetSize == 0)
            {
                entireDataset[entireIdx].push_back(data);
                ArrayVector[idx].m_datasetSize++;
                ArrayVector[idx].writeTimes++;
                ArrayVector[idx].SetDataset(entireDataset[entireIdx]);
                return true;
            }
            int preIdx = ArrayVector[idx].model.PredictPrecision(data.first, ArrayVector[idx].m_datasetSize);
            int start = max(0, preIdx - ArrayVector[idx].error);
            int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
            start = min(start, end);

            if (data.first <= entireDataset[entireIdx][start].first)
                preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, 0, start);
            else if (data.first <= entireDataset[entireIdx][end].first)
                preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, start, end);
            else
                preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, end, ArrayVector[idx].m_datasetSize - 1);

            // Insert data
            if (preIdx == ArrayVector[idx].m_datasetSize - 1 && entireDataset[entireIdx][preIdx].first < data.first)
            {
                entireDataset[entireIdx].push_back(data);
                ArrayVector[idx].m_datasetSize++;
                ArrayVector[idx].writeTimes++;
                return true;
            }
            entireDataset[entireIdx].push_back(entireDataset[entireIdx][ArrayVector[idx].m_datasetSize - 1]);
            ArrayVector[idx].m_datasetSize++;
            for (int i = ArrayVector[idx].m_datasetSize - 2; i > preIdx; i--)
                entireDataset[entireIdx][i] = entireDataset[entireIdx][i - 1];
            entireDataset[entireIdx][preIdx] = data;

            ArrayVector[idx].writeTimes++;

            // If the current number is greater than the maximum,
            // the child node needs to be retrained
            if (ArrayVector[idx].writeTimes >= ArrayVector[idx].m_datasetSize || ArrayVector[idx].writeTimes > ArrayVector[idx].m_maxNumber)
                ArrayVector[idx].SetDataset(entireDataset[entireIdx]);
            return true;
        }
        break;
        case 5:
        {
            auto entireIdx = GAVector[idx].datasetIndex;
            if ((float(GAVector[idx].m_datasetSize) / GAVector[idx].capacity > GAVector[idx].density))
            {
                // If an additional Insertion results in crossing the density
                // then we expand the gapped array
                GAVector[idx].SetDataset(entireDataset[entireIdx]);
            }

            if (GAVector[idx].m_datasetSize == 0)
            {
                entireDataset[entireIdx] = vector<pair<double, double>>(GAVector[idx].capacity, pair<double, double>{-1, -1});
                entireDataset[entireIdx][0] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = 0;
                GAVector[idx].SetDataset(entireDataset[entireIdx]);
                return true;
            }
            int preIdx = GAVector[idx].model.PredictPrecision(data.first, GAVector[idx].maxIndex + 1);

            int start = max(0, preIdx - GAVector[idx].error);
            int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
            start = min(start, end);

            if (entireDataset[entireIdx][start].first == -1)
                start--;
            if (entireDataset[entireIdx][end].first == -1)
                end--;

            if (data.first <= entireDataset[entireIdx][start].first)
                preIdx = GABinarySearch(entireDataset[entireIdx], data.first, 0, start);
            else if (data.first <= entireDataset[entireIdx][end].first)
                preIdx = GABinarySearch(entireDataset[entireIdx], data.first, start, end);
            else
                preIdx = GABinarySearch(entireDataset[entireIdx], data.first, end, GAVector[idx].maxIndex);

            // if the Insertion position is a gap,
            //  then we Insert the element into the gap and are done
            if (entireDataset[entireIdx][preIdx].first == -1)
            {
                entireDataset[entireIdx][preIdx] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx);
                return true;
            }
            else
            {
                if (entireDataset[entireIdx][preIdx].second == DBL_MIN)
                {
                    entireDataset[entireIdx][preIdx] = data;
                    GAVector[idx].m_datasetSize++;
                    GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx);
                    return true;
                }
                if (preIdx == GAVector[idx].maxIndex && entireDataset[entireIdx][GAVector[idx].maxIndex].first < data.first)
                {
                    entireDataset[entireIdx][++GAVector[idx].maxIndex] = data;
                    GAVector[idx].m_datasetSize++;
                    return true;
                }
                // If the Insertion position is not a gap, we make
                // a gap at the Insertion position by shifting the elements
                // by one position in the direction of the closest gap

                int i = preIdx + 1;
                while (entireDataset[entireIdx][i].first != -1)
                    i++;
                if (i >= GAVector[idx].capacity)
                {
                    i = preIdx - 1;
                    while (i >= 0 && entireDataset[entireIdx][i].first != -1)
                        i--;
                    for (int j = i; j < preIdx - 1; j++)
                        entireDataset[entireIdx][j] = entireDataset[entireIdx][j + 1];
                    preIdx--;
                }
                else
                {
                    if (i > GAVector[idx].maxIndex)
                        GAVector[idx].maxIndex++;
                    for (; i > preIdx; i--)
                        entireDataset[entireIdx][i] = entireDataset[entireIdx][i - 1];
                }
                entireDataset[entireIdx][preIdx] = data;
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
            auto entireIdx = ArrayVector[idx].datasetIndex;
            int preIdx = ArrayVector[idx].model.PredictPrecision(key, ArrayVector[idx].m_datasetSize);
            if (entireDataset[entireIdx][preIdx].first != key)
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                int res;
                if (key <= entireDataset[entireIdx][start].first)
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, 0, start);
                else if (key <= entireDataset[entireIdx][end].first)
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, start, end);
                else
                {
                    res = ArrayBinarySearch(entireDataset[entireIdx], key, end, ArrayVector[idx].m_datasetSize - 1);
                    if (res >= ArrayVector[idx].m_datasetSize)
                        return false;
                }
                if (entireDataset[entireIdx][res].first == key)
                    preIdx = res;
                else
                    return false;
            }
            for (int i = preIdx; i < ArrayVector[idx].m_datasetSize - 1; i++)
                entireDataset[entireIdx][i] = entireDataset[entireIdx][i + 1];
            ArrayVector[idx].m_datasetSize--;
            entireDataset[entireIdx].pop_back();
            ArrayVector[idx].writeTimes++;
            return true;
        }
        break;
        case 5:
        {
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            auto entireIdx = GAVector[idx].datasetIndex;
            int preIdx = GAVector[idx].model.PredictPrecision(key, GAVector[idx].maxIndex + 1);
            if (entireDataset[entireIdx][preIdx].first == key)
            {
                entireDataset[entireIdx][preIdx].second = DBL_MIN;
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
                if (entireDataset[entireIdx][start].first == -1)
                    start--;
                if (entireDataset[entireIdx][end].first == -1)
                    end--;
                if (key <= entireDataset[entireIdx][start].first)
                    res = GABinarySearch(entireDataset[entireIdx], key, 0, start);
                else if (key <= entireDataset[entireIdx][end].first)
                    res = GABinarySearch(entireDataset[entireIdx], key, start, end);
                else
                {
                    res = GABinarySearch(entireDataset[entireIdx], key, end, GAVector[idx].maxIndex);
                    if (res > GAVector[idx].maxIndex)
                        return false;
                }

                if (entireDataset[entireIdx][res].first != key)
                    return false;
                GAVector[idx].m_datasetSize--;
                entireDataset[entireIdx][res].second = DBL_MIN;
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
            auto entireIdx = ArrayVector[idx].datasetIndex;
            int preIdx = ArrayVector[idx].model.PredictPrecision(data.first, ArrayVector[idx].m_datasetSize);
            if (entireDataset[entireIdx][preIdx].first != data.first)
            {
                int start = max(0, preIdx - ArrayVector[idx].error);
                int end = min(ArrayVector[idx].m_datasetSize - 1, preIdx + ArrayVector[idx].error);
                start = min(start, end);
                if (data.first <= entireDataset[entireIdx][start].first)
                    preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, 0, start);
                else if (data.first <= entireDataset[entireIdx][end].first)
                    preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, start, end);
                else
                {
                    preIdx = ArrayBinarySearch(entireDataset[entireIdx], data.first, end, ArrayVector[idx].m_datasetSize - 1);
                    if (preIdx >= ArrayVector[idx].m_datasetSize)
                        return false;
                }
                if (entireDataset[entireIdx][preIdx].first != data.first)
                    return false;
            }
            entireDataset[entireIdx][preIdx].second = data.second;
            return true;
        }
        break;
        case 5:
        {
            auto entireIdx = GAVector[idx].datasetIndex;
            int preIdx = GAVector[idx].model.PredictPrecision(data.first, GAVector[idx].maxIndex + 1);
            if (entireDataset[entireIdx][preIdx].first == data.first)
            {
                entireDataset[entireIdx][preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - GAVector[idx].error);
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error);
                start = min(start, end);
                if (entireDataset[entireIdx][start].first == -1)
                    start--;
                if (entireDataset[entireIdx][end].first == -1)
                    end--;

                if (data.first <= entireDataset[entireIdx][start].first)
                    preIdx = GABinarySearch(entireDataset[entireIdx], data.first, 0, start);
                else if (data.first <= entireDataset[entireIdx][end].first)
                    preIdx = GABinarySearch(entireDataset[entireIdx], data.first, start, end);
                else
                {
                    preIdx = GABinarySearch(entireDataset[entireIdx], data.first, end, GAVector[idx].maxIndex);
                    if (preIdx > GAVector[idx].maxIndex)
                        return false;
                }

                if (entireDataset[entireIdx][preIdx].first != data.first)
                    return false;
                entireDataset[entireIdx][preIdx].second = data.second;
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
