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
#include "datapoint.h"
#include <float.h>
#include <vector>
#include <math.h>
#include <iomanip>
using namespace std;
 
vector<LRType> LRVector;
vector<NNType> NNVector;
vector<HisType> HisVector;
vector<BSType> BSVector;
vector<ArrayType> ArrayVector;
vector<GappedArrayType> GAVector;

extern pair<double, double> *entireData;
extern unsigned int entireDataSize;

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
            content = entireChild[LRVector[idx].childLeft + LRVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = entireChild[NNVector[idx].childLeft + NNVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = entireChild[HisVector[idx].childLeft + HisVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = entireChild[BSVector[idx].childLeft + BSVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto size = ArrayVector[idx].m_datasetSize;
            int preIdx = ArrayVector[idx].model.PredictPrecision(key, size);
            auto left = ArrayVector[idx].m_left;
            if (entireData[left + preIdx].first == key)
                return entireData[left + preIdx];
            else
            {
                int start = max(0, preIdx - ArrayVector[idx].error) + left;
                int end = min(size - 1, preIdx + ArrayVector[idx].error) + left;
                start = min(start, end);
                int res;
                if (key <= entireData[start].first)
                    res = ArrayBinarySearch(key, left, start);
                else if (key <= entireData[end].first)
                    res = ArrayBinarySearch(key, start, end);
                else
                {
                    res = ArrayBinarySearch(key, end, left + size - 1);
                    if (res >= left + size)
                        return {};
                }
                if (entireData[res].first == key)
                    return entireData[res];
                return {};
            }
        }
        break;
        case 5:
        {
            auto left = GAVector[idx].m_left;
            int preIdx = GAVector[idx].model.PredictPrecision(key, GAVector[idx].maxIndex + 1);
            if (entireData[left + preIdx].first == key)
                return entireData[left + preIdx];
            else
            {
                int start = max(0, preIdx - GAVector[idx].error) + left;
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error) + left;
                start = min(start, end);

                int res;
                if (entireData[start].first == -1)
                    start--;
                if (entireData[end].first == -1)
                    end--;
                if (key <= entireData[start].first)
                    res = GABinarySearch(key, left, start);
                else if (key <= entireData[end].first)
                    res = GABinarySearch(key, start, end);
                else
                {
                    res = GABinarySearch(key, end, left + GAVector[idx].maxIndex);
                    if (res > left + GAVector[idx].maxIndex)
                        return {DBL_MIN, DBL_MIN};
                }

                if (entireData[res].first == key)
                    return entireData[res];
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
            content = entireChild[LRVector[idx].childLeft + LRVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;

            // check split
            if (type == 4 && ArrayVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = LRVector[idx].childLeft + LRVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = ArrayVector[idx].m_left;
                auto size = ArrayVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                ArrayVector.erase(ArrayVector.begin() + idx); // delete the old leaf node
                auto node = LRType(128);                      // create a new inner node
                LRVector.push_back(node);
                idx = LRVector.size() - 1;
                entireChild[oldChildIdx] = 0x00000000 + idx;
                int childNum = LRVector[idx].childNumber;
                LRVector[idx].childLeft = allocateChildMemory(childNum);
                LRVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = LRVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    ArrayVector.push_back(ArrayType(kThreshold));
                    int leafIdx = ArrayVector.size() - 1;
                    ArrayVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[LRVector[idx].childLeft + i] = 0x40000000 + leafIdx;
                }
                type = 0;
            }
            else if (type == 5 && GAVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = LRVector[idx].childLeft + LRVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = GAVector[idx].m_left;
                auto size = GAVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                GAVector.erase(GAVector.begin() + idx); // delete the old leaf node
                auto node = LRType(128);                // create a new inner node
                LRVector.push_back(node);
                idx = LRVector.size() - 1;
                entireChild[oldChildIdx] = 0x00000000 + idx;
                int childNum = LRVector[idx].childNumber;
                LRVector[idx].childLeft = allocateChildMemory(childNum);
                LRVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = LRVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    GAVector.push_back(GappedArrayType(kThreshold));
                    int leafIdx = GAVector.size() - 1;
                    GAVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[LRVector[idx].childLeft + i] = 0x50000000 + leafIdx;
                }
                type = 0;
            }
        }
        break;
        case 1:
        {
            content = entireChild[NNVector[idx].childLeft + NNVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;

            // check split
            if (type == 4 && ArrayVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = NNVector[idx].childLeft + NNVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = ArrayVector[idx].m_left;
                auto size = ArrayVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                ArrayVector.erase(ArrayVector.begin() + idx); // delete the old leaf node
                auto node = NNType(128);                      // create a new inner node
                NNVector.push_back(node);
                idx = NNVector.size() - 1;
                entireChild[oldChildIdx] = 0x10000000 + idx;
                int childNum = NNVector[idx].childNumber;
                NNVector[idx].childLeft = allocateChildMemory(childNum);
                NNVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = NNVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    ArrayVector.push_back(ArrayType(kThreshold));
                    int leafIdx = ArrayVector.size() - 1;
                    ArrayVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[NNVector[idx].childLeft + i] = 0x40000000 + leafIdx;
                }
                type = 1;
            }
            else if (type == 5 && GAVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = NNVector[idx].childLeft + NNVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = GAVector[idx].m_left;
                auto size = GAVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                GAVector.erase(GAVector.begin() + idx); // delete the old leaf node
                auto node = NNType(128);                // create a new inner node
                NNVector.push_back(node);
                idx = NNVector.size() - 1;
                entireChild[oldChildIdx] = 0x10000000 + idx;
                int childNum = NNVector[idx].childNumber;
                NNVector[idx].childLeft = allocateChildMemory(childNum);
                NNVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = NNVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    GAVector.push_back(GappedArrayType(kThreshold));
                    int leafIdx = GAVector.size() - 1;
                    GAVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[NNVector[idx].childLeft + i] = 0x50000000 + leafIdx;
                }
                type = 1;
            }
        }
        break;
        case 2:
        {
            content = entireChild[HisVector[idx].childLeft + HisVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            // check split
            if (type == 4 && ArrayVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = HisVector[idx].childLeft + HisVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = ArrayVector[idx].m_left;
                auto size = ArrayVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                ArrayVector.erase(ArrayVector.begin() + idx); // delete the old leaf node
                auto node = HisType(128);                     // create a new inner node
                HisVector.push_back(node);
                idx = HisVector.size() - 1;
                entireChild[oldChildIdx] = 0x20000000 + idx;
                int childNum = HisVector[idx].childNumber;
                HisVector[idx].childLeft = allocateChildMemory(childNum);
                HisVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = HisVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    ArrayVector.push_back(ArrayType(kThreshold));
                    int leafIdx = ArrayVector.size() - 1;
                    ArrayVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[HisVector[idx].childLeft + i] = 0x40000000 + leafIdx;
                }
                type = 2;
            }
            else if (type == 5 && GAVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = HisVector[idx].childLeft + HisVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = GAVector[idx].m_left;
                auto size = GAVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                GAVector.erase(GAVector.begin() + idx); // delete the old leaf node
                auto node = HisType(128);               // create a new inner node
                HisVector.push_back(node);
                idx = HisVector.size() - 1;
                entireChild[oldChildIdx] = 0x20000000 + idx;
                int childNum = HisVector[idx].childNumber;
                HisVector[idx].childLeft = allocateChildMemory(childNum);
                HisVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = HisVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    GAVector.push_back(GappedArrayType(kThreshold));
                    int leafIdx = GAVector.size() - 1;
                    GAVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[HisVector[idx].childLeft + i] = 0x50000000 + leafIdx;
                }
                type = 2;
            }
        }
        break;
        case 3:
        {
            content = entireChild[BSVector[idx].childLeft + BSVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            // check split
            if (type == 4 && ArrayVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = BSVector[idx].childLeft + BSVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = ArrayVector[idx].m_left;
                auto size = ArrayVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                ArrayVector.erase(ArrayVector.begin() + idx); // delete the old leaf node
                auto node = BSType(128);                      // create a new inner node
                BSVector.push_back(node);
                idx = BSVector.size() - 1;
                entireChild[oldChildIdx] = 0x30000000 + idx;
                int childNum = BSVector[idx].childNumber;
                BSVector[idx].childLeft = allocateChildMemory(childNum);
                BSVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = BSVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    ArrayVector.push_back(ArrayType(kThreshold));
                    int leafIdx = ArrayVector.size() - 1;
                    ArrayVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[BSVector[idx].childLeft + i] = 0x40000000 + leafIdx;
                }
                type = 3;
            }
            else if (type == 5 && GAVector[idx].m_datasetSize >= 4096)
            {
                int oldChildIdx = BSVector[idx].childLeft + BSVector[idx].model.Predict(data.first); // idx in entireChild
                vector<pair<double, double>> tmpDataset;
                auto left = GAVector[idx].m_left;
                auto size = GAVector[idx].m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);
                GAVector.erase(GAVector.begin() + idx); // delete the old leaf node
                auto node = BSType(128);                // create a new inner node
                BSVector.push_back(node);
                idx = BSVector.size() - 1;
                entireChild[oldChildIdx] = 0x30000000 + idx;
                int childNum = BSVector[idx].childNumber;
                BSVector[idx].childLeft = allocateChildMemory(childNum);
                BSVector[idx].model.Train(tmpDataset, childNum);

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = BSVector[idx].model.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    GAVector.push_back(GappedArrayType(kThreshold));
                    int leafIdx = GAVector.size() - 1;
                    GAVector[leafIdx].SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[BSVector[idx].childLeft + i] = 0x50000000 + leafIdx;
                }
                type = 3;
            }
        }
        break;
        case 4:
        {
            auto left = ArrayVector[idx].m_left;
            if (ArrayVector[idx].m_datasetSize == 0)
            {
                entireData[left] = data;
                ArrayVector[idx].m_datasetSize++;
                ArrayVector[idx].SetDataset(left, 1, ArrayVector[idx].m_capacity);
                return true;
            }
            auto size = ArrayVector[idx].m_datasetSize;
            int preIdx = ArrayVector[idx].model.PredictPrecision(data.first, size);
            int start = max(0, preIdx - ArrayVector[idx].error) + left;
            int end = min(size - 1, preIdx + ArrayVector[idx].error) + left;
            start = min(start, end);

            if (data.first <= entireData[start].first)
                preIdx = ArrayBinarySearch(data.first, left, start);
            else if (data.first <= entireData[end].first)
                preIdx = ArrayBinarySearch(data.first, start, end);
            else
                preIdx = ArrayBinarySearch(data.first, end, left + size - 1);

            // expand
            if (size >= ArrayVector[idx].m_capacity)
            {
                auto diff = preIdx - left;
                ArrayVector[idx].SetDataset(left, size, ArrayVector[idx].m_capacity);
                left = ArrayVector[idx].m_left;
                preIdx = left + diff;
            }

            // Insert data
            if ((preIdx == left + size - 1) && (entireData[preIdx].first < data.first))
            {
                entireData[left + size] = data;
                ArrayVector[idx].m_datasetSize++;
                return true;
            }
            ArrayVector[idx].m_datasetSize++;
            for (int i = left + size; i > preIdx; i--)
                entireData[i] = entireData[i - 1];
            entireData[preIdx] = data;
            return true;
        }
        break;
        case 5:
        {
            if (idx >= GAVector.size())
                cout << "idx exceed the size of GAVector!" << endl;
            auto left = GAVector[idx].m_left;
            if ((float(GAVector[idx].m_datasetSize) / GAVector[idx].capacity > GAVector[idx].density))
            {
                // If an additional Insertion results in crossing the density
                // then we expand the gapped array
                GAVector[idx].SetDataset(left, GAVector[idx].m_datasetSize, GAVector[idx].capacity);
                left = GAVector[idx].m_left;
            }

            if (GAVector[idx].m_datasetSize == 0)
            {
                entireData[left] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = 0;
                GAVector[idx].SetDataset(left, 1, GAVector[idx].capacity);
                return true;
            }
            int preIdx = GAVector[idx].model.PredictPrecision(data.first, GAVector[idx].maxIndex + 1);

            int start = max(0, preIdx - GAVector[idx].error) + left;
            int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error) + left;
            start = min(start, end);

            if (entireData[start].first == -1)
                start--;
            if (entireData[end].first == -1)
                end--;

            if (data.first <= entireData[start].first)
                preIdx = GABinarySearch(data.first, left, start);
            else if (data.first <= entireData[end].first)
                preIdx = GABinarySearch(data.first, start, end);
            else
                preIdx = GABinarySearch(data.first, end, left + GAVector[idx].maxIndex);

            // if the Insertion position is a gap,
            //  then we Insert the element into the gap and are done
            if (entireData[preIdx].first == -1)
            {
                entireData[preIdx] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx - left);
                return true;
            }
            else
            {
                if (entireData[preIdx].second == DBL_MIN)
                {
                    entireData[preIdx] = data;
                    GAVector[idx].m_datasetSize++;
                    GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx - left);
                    return true;
                }
                if (preIdx == left + GAVector[idx].maxIndex && entireData[left + GAVector[idx].maxIndex].first < data.first)
                {
                    GAVector[idx].maxIndex = GAVector[idx].maxIndex + 1;
                    ;
                    entireData[GAVector[idx].maxIndex + left] = data;
                    GAVector[idx].m_datasetSize++;
                    return true;
                }

                // If the Insertion position is not a gap, we make
                // a gap at the Insertion position by shifting the elements
                // by one position in the direction of the closest gap
                int i = preIdx + 1;
                while (entireData[i].first != -1)
                    i++;
                if (i >= left + GAVector[idx].capacity)
                {
                    i = preIdx - 1;
                    while (i >= left && entireData[i].first != -1)
                        i--;
                    for (int j = i; j < preIdx - 1; j++)
                        entireData[j] = entireData[j + 1];
                    preIdx--;
                }
                else
                {
                    if (i > GAVector[idx].maxIndex + left)
                        GAVector[idx].maxIndex++;
                    for (; i > preIdx; i--)
                        entireData[i] = entireData[i - 1];
                }
                entireData[preIdx] = data;
                GAVector[idx].m_datasetSize++;
                GAVector[idx].maxIndex = max(GAVector[idx].maxIndex, preIdx - left);
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
            content = entireChild[LRVector[idx].childLeft + LRVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = entireChild[NNVector[idx].childLeft + NNVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = entireChild[HisVector[idx].childLeft + HisVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = entireChild[BSVector[idx].childLeft + BSVector[idx].model.Predict(key)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto left = ArrayVector[idx].m_left;
            auto size = ArrayVector[idx].m_datasetSize;
            int preIdx = ArrayVector[idx].model.PredictPrecision(key, size);
            if (entireData[left + preIdx].first == key)
                preIdx += left;
            else
            {
                int start = max(0, preIdx - ArrayVector[idx].error) + left;
                int end = min(size - 1, preIdx + ArrayVector[idx].error) + left;
                start = min(start, end);
                int res;
                if (key <= entireData[start].first)
                    res = ArrayBinarySearch(key, left, start);
                else if (key <= entireData[end].first)
                    res = ArrayBinarySearch(key, start, end);
                else
                {
                    res = ArrayBinarySearch(key, end, left + size - 1);
                    if (res >= left + size)
                        return false;
                }
                if (entireData[res].first == key)
                    preIdx = res;
                else
                    return false;
            }
            for (int i = preIdx; i < left + size - 1; i++)
                entireData[i] = entireData[i + 1];
            entireData[left + size - 1] = {DBL_MIN, DBL_MIN};
            ArrayVector[idx].m_datasetSize--;
            return true;
        }
        break;
        case 5:
        {
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            auto left = GAVector[idx].m_left;
            int preIdx = GAVector[idx].model.PredictPrecision(key, GAVector[idx].maxIndex + 1);
            if (entireData[left + preIdx].first == key)
            {
                entireData[left + preIdx].second = DBL_MIN;
                GAVector[idx].m_datasetSize--;
                if (preIdx == GAVector[idx].maxIndex)
                    GAVector[idx].maxIndex--;
                return true;
            }
            else
            {
                int start = max(0, preIdx - GAVector[idx].error) + left;
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error) + left;
                start = min(start, end);

                int res;
                if (entireData[start].first == -1)
                    start--;
                if (entireData[end].first == -1)
                    end--;
                if (key <= entireData[start].first)
                    res = GABinarySearch(key, left, start);
                else if (key <= entireData[end].first)
                    res = GABinarySearch(key, start, end);
                else
                {
                    res = GABinarySearch(key, end, left + GAVector[idx].maxIndex);
                    if (res > left + GAVector[idx].maxIndex)
                        return false;
                }

                if (entireData[res].first != key)
                    return false;
                GAVector[idx].m_datasetSize--;
                entireData[res] = {DBL_MIN, DBL_MIN};
                if (res == left + GAVector[idx].maxIndex)
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
            content = entireChild[LRVector[idx].childLeft + LRVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 1:
        {
            content = entireChild[NNVector[idx].childLeft + NNVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 2:
        {
            content = entireChild[HisVector[idx].childLeft + HisVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 3:
        {
            content = entireChild[BSVector[idx].childLeft + BSVector[idx].model.Predict(data.first)];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
        }
        break;
        case 4:
        {
            auto left = ArrayVector[idx].m_left;
            auto size = ArrayVector[idx].m_datasetSize;
            int preIdx = ArrayVector[idx].model.PredictPrecision(data.first, size);
            if (entireData[left + preIdx].first == data.first)
                entireData[left + preIdx].second = data.second;
            else
            {
                int start = max(0, preIdx - ArrayVector[idx].error) + left;
                int end = min(size - 1, preIdx + ArrayVector[idx].error) + left;
                start = min(start, end);
                if (data.first <= entireData[start].first)
                    preIdx = ArrayBinarySearch(data.first, left, start);
                else if (data.first <= entireData[end].first)
                    preIdx = ArrayBinarySearch(data.first, start, end);
                else
                {
                    preIdx = ArrayBinarySearch(data.first, end, left + size - 1);
                    if (preIdx >= left + size)
                        return false;
                }
                if (entireData[preIdx].first != data.first)
                    return false;
                entireData[preIdx].second = data.second;
            }
            return true;
        }
        break;
        case 5:
        {
            auto left = GAVector[idx].m_left;
            int preIdx = GAVector[idx].model.PredictPrecision(data.first, GAVector[idx].maxIndex + 1);
            if (entireData[left + preIdx].first == data.first)
            {
                entireData[left + preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - GAVector[idx].error) + left;
                int end = min(GAVector[idx].maxIndex, preIdx + GAVector[idx].error) + left;
                start = min(start, end);
                if (entireData[start].first == -1)
                    start--;
                if (entireData[end].first == -1)
                    end--;

                if (data.first <= entireData[start].first)
                    preIdx = GABinarySearch(data.first, left, start);
                else if (data.first <= entireData[end].first)
                    preIdx = GABinarySearch(data.first, start, end);
                else
                {
                    preIdx = GABinarySearch(data.first, end, left + GAVector[idx].maxIndex);
                    if (preIdx > left + GAVector[idx].maxIndex)
                        return false;
                }

                if (entireData[preIdx].first != data.first)
                    return false;
                entireData[preIdx].second = data.second;
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
