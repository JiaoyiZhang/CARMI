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
#include <vector>
using namespace std;

vector<void *> INDEX;  // store the entire INDEX

extern int kLeafNodeID;
extern int kInnerNodeID;

extern int kThreshold;
extern const double kDensity;
extern const int kAdaptiveChildNum;
extern double kRate;
extern const double kReadWriteRate;
extern int kMaxKeyNum;

void Initialize(const vector<pair<double, double>> &dataset, int childNum)
{
    // create the root node
    switch (kInnerNodeID)
    {
    case 0:
        INDEX.push_back((void *)new LRType(childNum));
        ((LRType *)INDEX[0])->Initialize(dataset, childNum);
        break;
    case 1:
        INDEX.push_back((void *)new NNType(childNum));
        ((NNType *)INDEX[0])->Initialize(dataset, childNum);
        break;
    case 2:
        INDEX.push_back((void *)new HisType(childNum));
        ((HisType *)INDEX[0])->Initialize(dataset, childNum);
        break;
    case 3:
        INDEX.push_back((void *)new BSType(childNum));
        ((BSType *)INDEX[0])->Initialize(dataset, childNum);
        break;
    }
}

pair<double, double> Find(double key)
{
    double p;
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    void * currentNode = INDEX[0];
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            p = ((LRType *)currentNode)->model.Predict(key);
            content = ((LRType *)currentNode)->child[static_cast<int>(p * (((LRType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 1:
        {
            p = ((NNType *)currentNode)->model.Predict(key);
            content = ((NNType *)currentNode)->child[static_cast<int>(p * (((NNType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 2:
        {
            p = ((HisType *)currentNode)->model.Predict(key);
            content = ((HisType *)currentNode)->child[static_cast<int>(p * (((HisType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];

        }
        break;
        case 3:
        {
            p = ((BSType *)currentNode)->model.Predict(key);
            content = ((BSType *)currentNode)->child[static_cast<int>(p * (((BSType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];

        }
        break;
        case 4:
        {
            auto leaf = (ArrayType *)currentNode;
            if (leaf->m_datasetSize == 0)
                return {};
            p = leaf->model.Predict(key);
            idx = static_cast<int>(p * (leaf->m_datasetSize - 1));
            if (leaf->m_dataset[idx].first == key)
            {
                return leaf->m_dataset[idx];
            }
            else
            {
                int start = max(0, idx - leaf->error);
                int end = min(leaf->m_datasetSize - 1, idx + leaf->error);
                start = min(start, end);
                int res = ArrayBinarySearch(leaf->m_dataset, key, idx, start, end);
                if (res <= start)
                    res = ArrayBinarySearch(leaf->m_dataset, key, idx, 0, start);
                else if (res >= end)
                {
                    res = ArrayBinarySearch(leaf->m_dataset, key, idx, res, leaf->m_datasetSize - 1);
                    if (res >= leaf->m_datasetSize)
                        return {};
                }
                if (leaf->m_dataset[res].first == key)
                    return leaf->m_dataset[res];
                return {};
            }

        }
        break;
        case 5:
        {
            auto galeaf = (GappedArrayType *)currentNode;
            p = galeaf->model.Predict(key);
            idx = static_cast<int>(p * (galeaf->capacity - 1));
            if (galeaf->m_dataset[idx].first == key)
                return galeaf->m_dataset[idx];
            else
            {
                int start = max(0, idx - galeaf->error);
                int end = min(galeaf->maxIndex, idx + galeaf->error);
                start = min(start, end);
                int res = GABinarySearch(galeaf->m_dataset, key, idx, start, end);

                if (res <= start)
                    res = GABinarySearch(galeaf->m_dataset, key, idx, 0, start);
                else if (res >= end)
                {
                    res = GABinarySearch(galeaf->m_dataset, key, idx, res, galeaf->maxIndex);
                    if (res > galeaf->maxIndex)
                        return {DBL_MIN, DBL_MIN};
                }
                if (galeaf->m_dataset[res].first == key)
                    return galeaf->m_dataset[res];
                return {DBL_MIN, DBL_MIN};
            }
        }
        break;
        }
    }
}

bool Insert(pair<double, double> data)
{
    double p;
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    void * currentNode = INDEX[0];
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            p = ((LRType *)currentNode)->model.Predict(data.first);
            content = ((LRType *)currentNode)->child[static_cast<int>(p * (((LRType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 1:
        {
            p = ((NNType *)currentNode)->model.Predict(data.first);
            content = ((NNType *)currentNode)->child[static_cast<int>(p * (((NNType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 2:
        {
            p = ((HisType *)currentNode)->model.Predict(data.first);
            content = ((HisType *)currentNode)->child[static_cast<int>(p * (((HisType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 3:
        {
            p = ((BSType *)currentNode)->model.Predict(data.first);
            content = ((BSType *)currentNode)->child[static_cast<int>(p * (((BSType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 4:
        {
            auto leaf = (ArrayType *)currentNode;
            if (leaf->m_datasetSize == 0)
            {
                leaf->m_dataset.push_back(data);
                leaf->m_datasetSize++;
                leaf->writeTimes++;
                leaf->SetDataset(leaf->m_dataset);
                return true;
            }
            p = leaf->model.Predict(data.first);
            idx = static_cast<int>(p * leaf->m_datasetSize - 1);
            int start = max(0, idx - leaf->error);
            int end = min(leaf->m_datasetSize - 1, idx + leaf->error);
            start = min(start, end);

            idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, start, end);
            if (idx <= start)
                idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, 0, start);
            else if (idx >= end)
                idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, idx, leaf->m_datasetSize - 1);

            // Insert data
            if (idx == leaf->m_datasetSize - 1 && leaf->m_dataset[idx].first < data.first)
            {
                leaf->m_dataset.push_back(data);
                leaf->m_datasetSize++;
                leaf->writeTimes++;
                return true;
            }
            leaf->m_dataset.push_back(leaf->m_dataset[leaf->m_datasetSize - 1]);
            leaf->m_datasetSize++;
            for (int i = leaf->m_datasetSize - 2; i > idx; i--)
                leaf->m_dataset[i] = leaf->m_dataset[i - 1];
            leaf->m_dataset[idx] = data;

            leaf->writeTimes++;

            // If the current number is greater than the maximum,
            // the child node needs to be retrained
            if (leaf->writeTimes >= leaf->m_datasetSize || leaf->writeTimes > leaf->m_maxNumber)
                leaf->SetDataset(leaf->m_dataset);
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = (GappedArrayType *)currentNode;
            if ((float(galeaf->m_datasetSize) / galeaf->capacity > galeaf->density))
                {
                    // If an additional Insertion results in crossing the density
                    // then we expand the gapped array
                    galeaf->SetDataset(galeaf->m_dataset);
                }

                if (galeaf->m_datasetSize == 0)
                {
                    galeaf->m_dataset = vector<pair<double, double>>(galeaf->capacity, pair<double, double>{-1, -1});
                    galeaf->m_dataset[0] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = 0;
                    galeaf->SetDataset(galeaf->m_dataset);
                    return true;
                }
                p = galeaf->model.Predict(data.first);
                idx = static_cast<int>(p * (galeaf->maxIndex + 2));

                int start = max(0, idx - galeaf->error);
                int end = min(galeaf->maxIndex, idx + galeaf->error);
                start = min(start, end);
                idx = GABinarySearch(galeaf->m_dataset, data.first, idx, start, end);
                if (idx <= start)
                    idx = GABinarySearch(galeaf->m_dataset, data.first, idx, 0, start);
                else if (idx >= end)
                    idx = GABinarySearch(galeaf->m_dataset, data.first, idx, idx, galeaf->maxIndex);

                // if the Insertion position is a gap,
                //  then we Insert the element into the gap and are done
                if (galeaf->m_dataset[idx].first == -1)
                {
                    galeaf->m_dataset[idx] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = max(galeaf->maxIndex, idx);
                    return true;
                }
                else
                {
                    if (galeaf->m_dataset[idx].second == DBL_MIN)
                    {
                        galeaf->m_dataset[idx] = data;
                        galeaf->m_datasetSize++;
                        galeaf->maxIndex = max(galeaf->maxIndex, idx);
                        return true;
                    }
                    if (idx == galeaf->maxIndex && galeaf->m_dataset[galeaf->maxIndex].first < data.first)
                    {
                        galeaf->m_dataset[++galeaf->maxIndex] = data;
                        galeaf->m_datasetSize++;
                        return true;
                    }
                    // If the Insertion position is not a gap, we make
                    // a gap at the Insertion position by shifting the elements
                    // by one position in the direction of the closest gap

                    int i = idx + 1;
                    while (galeaf->m_dataset[i].first != -1)
                        i++;
                    if (i >= galeaf->capacity)
                    {
                        i = idx - 1;
                        while (i >= 0 && galeaf->m_dataset[i].first != -1)
                            i--;
                        for (int j = i; j < idx - 1; j++)
                            galeaf->m_dataset[j] = galeaf->m_dataset[j + 1];
                        idx--;
                    }
                    else
                    {
                        if (i > galeaf->maxIndex)
                            galeaf->maxIndex++;
                        for (; i > idx; i--)
                            galeaf->m_dataset[i] = galeaf->m_dataset[i - 1];
                    }
                    galeaf->m_dataset[idx] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = max(galeaf->maxIndex, idx);
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
    double p;
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    void * currentNode = INDEX[0];
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            p = ((LRType *)currentNode)->model.Predict(key);
            content = ((LRType *)currentNode)->child[static_cast<int>(p * (((LRType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 1:
        {
            p = ((NNType *)currentNode)->model.Predict(key);
            content = ((NNType *)currentNode)->child[static_cast<int>(p * (((NNType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 2:
        {
            p = ((HisType *)currentNode)->model.Predict(key);
            content = ((HisType *)currentNode)->child[static_cast<int>(p * (((HisType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 3:
        {
            p = ((BSType *)currentNode)->model.Predict(key);
            content = ((BSType *)currentNode)->child[static_cast<int>(p * (((BSType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 4:
        {
            auto leaf = (ArrayType *)currentNode;
            p = leaf->model.Predict(key);
            idx = static_cast<int>(p * (leaf->m_datasetSize - 1));
            if (leaf->m_dataset[idx].first != key)
            {
                int start = max(0, idx - leaf->error);
                int end = min(leaf->m_datasetSize - 1, idx + leaf->error);
                start = min(start, end);
                int res = ArrayBinarySearch(leaf->m_dataset, key, idx, start, end);
                if (leaf->m_dataset[res].first == key)
                    idx = res;
                else
                {
                    if (res <= start)
                        res = ArrayBinarySearch(leaf->m_dataset, key, idx, 0, start);
                    else if (res >= end)
                    {
                        res = ArrayBinarySearch(leaf->m_dataset, key, idx, res, leaf->m_datasetSize - 1);
                        if (res >= leaf->m_datasetSize)
                            return false;
                    }
                    if (leaf->m_dataset[res].first == key)
                        idx = res;
                    else
                        return false;
                }
            }
            for (int i = idx; i < leaf->m_datasetSize - 1; i++)
                leaf->m_dataset[i] = leaf->m_dataset[i + 1];
            leaf->m_datasetSize--;
            leaf->m_dataset.pop_back();
            leaf->writeTimes++;
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = (GappedArrayType *)currentNode;
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            p = galeaf->model.Predict(key);
            idx = static_cast<int>(p * (galeaf->capacity - 1));
            if (galeaf->m_dataset[idx].first == key)
            {
                galeaf->m_dataset[idx].second = DBL_MIN;
                galeaf->m_datasetSize--;
                if (idx == galeaf->maxIndex)
                    galeaf->maxIndex--;
                return true;
            }
            else
            {
                int start = max(0, idx - galeaf->error);
                int end = min(galeaf->maxIndex, idx + galeaf->error);
                start = min(start, end);
                int res = GABinarySearch(galeaf->m_dataset, key, idx, start, end);

                if (res <= start)
                    res = GABinarySearch(galeaf->m_dataset, key, idx, 0, start);
                else if (res >= end)
                {
                    res = GABinarySearch(galeaf->m_dataset, key, idx, res, galeaf->maxIndex);
                    if (res > galeaf->maxIndex)
                        return false;
                }
                if (galeaf->m_dataset[res].first != key)
                    return false;
                galeaf->m_datasetSize--;
                galeaf->m_dataset[res].second = DBL_MIN;
                if (res == galeaf->maxIndex)
                    galeaf->maxIndex--;
                return true;
            }
        }
        break;
        }
    }
}

bool Update(pair<double, double> data)
{
    double p;
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    void * currentNode = INDEX[0];
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            p = ((LRType *)currentNode)->model.Predict(data.first);
            content = ((LRType *)currentNode)->child[static_cast<int>(p * (((LRType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 1:
        {
            p = ((NNType *)currentNode)->model.Predict(data.first);
            content = ((NNType *)currentNode)->child[static_cast<int>(p * (((NNType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 2:
        {
            p = ((HisType *)currentNode)->model.Predict(data.first);
            content = ((HisType *)currentNode)->child[static_cast<int>(p * (((HisType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 3:
        {
            p = ((BSType *)currentNode)->model.Predict(data.first);
            content = ((BSType *)currentNode)->child[static_cast<int>(p * (((BSType *)currentNode)->childNumber - 1))];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            currentNode = INDEX[idx];
        }
        break;
        case 4:
        {
            auto leaf = (ArrayType *)currentNode;
            p = leaf->model.Predict(data.first);
            idx = static_cast<int>(p * (leaf->m_datasetSize - 1));
            if (leaf->m_dataset[idx].first != data.first)
            {
                int start = max(0, idx - leaf->error);
                int end = min(leaf->m_datasetSize - 1, idx + leaf->error);
                start = min(start, end);
                int res = ArrayBinarySearch(leaf->m_dataset, data.first, idx, start, end);
                if (leaf->m_dataset[res].first == data.first)
                    idx = res;
                else
                {
                    if (res <= start)
                        res = ArrayBinarySearch(leaf->m_dataset, data.first, idx, 0, start);
                    else if (res >= end)
                    {
                        res = ArrayBinarySearch(leaf->m_dataset, data.first, idx, res, leaf->m_datasetSize - 1);
                        if (res >= leaf->m_datasetSize)
                            return false;
                    }
                    if (leaf->m_dataset[res].first == data.first)
                        idx = res;
                    else
                        return false;
                }
            }
            leaf->m_dataset[idx].second = data.second;
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = (GappedArrayType *)currentNode;
            p = galeaf->model.Predict(data.first);
            idx = static_cast<int>(p * (galeaf->capacity - 1));
            if (galeaf->m_dataset[idx].first == data.first)
            {
                galeaf->m_dataset[idx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, idx - galeaf->error);
                int end = min(galeaf->maxIndex, idx + galeaf->error);
                start = min(start, end);
                int res = GABinarySearch(galeaf->m_dataset, data.first, idx, start, end);
                if (res <= start)
                    res = GABinarySearch(galeaf->m_dataset, data.first, idx, 0, start);
                else if (res >= end)
                {
                    res = GABinarySearch(galeaf->m_dataset, data.first, idx, res, galeaf->maxIndex);
                    if (res > galeaf->maxIndex)
                        return false;
                }
                if (galeaf->m_dataset[res].first != data.first)
                    return false;
                galeaf->m_dataset[res].second = data.second;
                return true;
            }
        }
        break;
        }
    }
}


bool AdaptiveInsert(pair<double, double> data)
{
    double p;
    int idx = 0;  // idx in the INDEX
    int content;
    int type = kInnerNodeID;
    void * currentNode = INDEX[0];
    while(1)
    {
        switch (type)
        {
        case 0:
        {
            p = ((LRType *)currentNode)->model.Predict(data.first);
            int tmpIdx = static_cast<int>(p * (((LRType *)currentNode)->childNumber - 1));
            content = ((LRType *)currentNode)->child[tmpIdx];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            
            // Check whether the next leaf node needs to be 
            // converted, if necessary, replace the node with
            // an inner node, and generate some new leaf nodes
            if(type == 4)
            {
                type = CheckArray(idx);
                // update the idx, currentNode, type
                ((LRType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            else if(type == 5)
            {
                type = CheckGappedArray(idx);
                // update the idx, currentNode, type
                ((LRType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            currentNode = INDEX[idx];
        }
        break;
        case 1:
        {
            p = ((NNType *)currentNode)->model.Predict(data.first);
            int tmpIdx = static_cast<int>(p * (((NNType *)currentNode)->childNumber - 1));
            content = ((NNType *)currentNode)->child[tmpIdx];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            
            if(type == 4)
            {
                type = CheckArray(idx);
                ((NNType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            else if(type == 5)
            {
                type = CheckGappedArray(idx);
                ((NNType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            currentNode = INDEX[idx];
        }
        break;
        case 2:
        {
            p = ((HisType *)currentNode)->model.Predict(data.first);
            int tmpIdx = static_cast<int>(p * (((HisType *)currentNode)->childNumber - 1));
            content = ((HisType *)currentNode)->child[tmpIdx];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            
            if(type == 4)
            {
                type = CheckArray(idx);
                ((HisType *)currentNode)->child[tmpIdx] = (type << 28)  + idx;
            }
            else if(type == 5)
            {
                type = CheckGappedArray(idx);
                ((HisType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            currentNode = INDEX[idx];
        }
        break;
        case 3:
        {
            p = ((BSType *)currentNode)->model.Predict(data.first);
            int tmpIdx = static_cast<int>(p * (((BSType *)currentNode)->childNumber - 1));
            content = ((BSType *)currentNode)->child[tmpIdx];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            
            if(type == 4)
            {
                type = CheckArray(idx);
                ((BSType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            else if(type == 5)
            {
                type = CheckGappedArray(idx);
                ((BSType *)currentNode)->child[tmpIdx] = (type << 28) + idx;
            }
            currentNode = INDEX[idx];
        }
        break;
        case 4:
        {
            auto leaf = (ArrayType *)currentNode;
            if (leaf->m_datasetSize == 0)
            {
                leaf->m_dataset.push_back(data);
                leaf->m_datasetSize++;
                leaf->writeTimes++;
                leaf->SetDataset(leaf->m_dataset);
                return true;
            }
            p = leaf->model.Predict(data.first);
            idx = static_cast<int>(p * leaf->m_datasetSize - 1);
            int start = max(0, idx - leaf->error);
            int end = min(leaf->m_datasetSize - 1, idx + leaf->error);
            start = min(start, end);

            idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, start, end);
            if (idx <= start)
                idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, 0, start);
            else if (idx >= end)
                idx = ArrayBinarySearch(leaf->m_dataset, data.first, idx, idx, leaf->m_datasetSize - 1);

            // Insert data
            if (idx == leaf->m_datasetSize - 1 && leaf->m_dataset[idx].first < data.first)
            {
                leaf->m_dataset.push_back(data);
                leaf->m_datasetSize++;
                leaf->writeTimes++;
                return true;
            }
            leaf->m_dataset.push_back(leaf->m_dataset[leaf->m_datasetSize - 1]);
            leaf->m_datasetSize++;
            for (int i = leaf->m_datasetSize - 2; i > idx; i--)
                leaf->m_dataset[i] = leaf->m_dataset[i - 1];
            leaf->m_dataset[idx] = data;

            leaf->writeTimes++;

            // If the current number is greater than the maximum,
            // the child node needs to be retrained
            if (leaf->writeTimes >= leaf->m_datasetSize || leaf->writeTimes > leaf->m_maxNumber)
                leaf->SetDataset(leaf->m_dataset);
            return true;
        }
        break;
        case 5:
        {
            auto galeaf = (GappedArrayType *)currentNode;
            if ((float(galeaf->m_datasetSize) / galeaf->capacity > galeaf->density))
                {
                    // If an additional Insertion results in crossing the density
                    // then we expand the gapped array
                    galeaf->SetDataset(galeaf->m_dataset);
                }

                if (galeaf->m_datasetSize == 0)
                {
                    galeaf->m_dataset = vector<pair<double, double>>(galeaf->capacity, pair<double, double>{-1, -1});
                    galeaf->m_dataset[0] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = 0;
                    galeaf->SetDataset(galeaf->m_dataset);
                    return true;
                }
                p = galeaf->model.Predict(data.first);
                idx = static_cast<int>(p * (galeaf->maxIndex + 2));

                int start = max(0, idx - galeaf->error);
                int end = min(galeaf->maxIndex, idx + galeaf->error);
                start = min(start, end);
                idx = GABinarySearch(galeaf->m_dataset, data.first, idx, start, end);
                if (idx <= start)
                    idx = GABinarySearch(galeaf->m_dataset, data.first, idx, 0, start);
                else if (idx >= end)
                    idx = GABinarySearch(galeaf->m_dataset, data.first, idx, idx, galeaf->maxIndex);

                // if the Insertion position is a gap,
                //  then we Insert the element into the gap and are done
                if (galeaf->m_dataset[idx].first == -1)
                {
                    galeaf->m_dataset[idx] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = max(galeaf->maxIndex, idx);
                    return true;
                }
                else
                {
                    if (galeaf->m_dataset[idx].second == DBL_MIN)
                    {
                        galeaf->m_dataset[idx] = data;
                        galeaf->m_datasetSize++;
                        galeaf->maxIndex = max(galeaf->maxIndex, idx);
                        return true;
                    }
                    if (idx == galeaf->maxIndex && galeaf->m_dataset[galeaf->maxIndex].first < data.first)
                    {
                        galeaf->m_dataset[++galeaf->maxIndex] = data;
                        galeaf->m_datasetSize++;
                        return true;
                    }
                    // If the Insertion position is not a gap, we make
                    // a gap at the Insertion position by shifting the elements
                    // by one position in the direction of the closest gap

                    int i = idx + 1;
                    while (galeaf->m_dataset[i].first != -1)
                        i++;
                    if (i >= galeaf->capacity)
                    {
                        i = idx - 1;
                        while (i >= 0 && galeaf->m_dataset[i].first != -1)
                            i--;
                        for (int j = i; j < idx - 1; j++)
                            galeaf->m_dataset[j] = galeaf->m_dataset[j + 1];
                        idx--;
                    }
                    else
                    {
                        if (i > galeaf->maxIndex)
                            galeaf->maxIndex++;
                        for (; i > idx; i--)
                            galeaf->m_dataset[i] = galeaf->m_dataset[i - 1];
                    }
                    galeaf->m_dataset[idx] = data;
                    galeaf->m_datasetSize++;
                    galeaf->maxIndex = max(galeaf->maxIndex, idx);
                    return true;
                }
                return false;
        }
        break;
        }
    }
}


void AdaptiveInitialize(const vector<pair<double, double>> &dataset, int childNum)
{
    if (dataset.size() == 0)
        return;

    cout << "train adaptive root node!" << endl;
    // create the root node
    switch (kInnerNodeID)
    {
    case 0:
    {
        INDEX.push_back((void *)new LRType(childNum));
        ((LRType *)INDEX[0])->model.Train(dataset);
        auto root = (LRType *)INDEX[0];
        switch (kLeafNodeID)
        {
            case 0:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new ArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x40000000 + idx);
                }
                break;
            case 1:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new GappedArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x50000000 + idx);
                }
                break;
        }
    }
    break;
    case 1:
    {
        INDEX.push_back((void *)new NNType(childNum));
        cout<<"Now index size: "<<INDEX.size()<<endl;
        ((NNType *)INDEX[0])->model.Train(dataset);
        cout<<"root train over!"<<endl;
        auto root = (NNType *)INDEX[0];
        switch (kLeafNodeID)
        {
            case 0:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new ArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x40000000 + idx);
                }
                break;
            case 1:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new GappedArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x50000000 + idx);
                }
                break;
        }
    }
    break;
    case 2:
    {
        INDEX.push_back((void *)new HisType(childNum));
        ((HisType *)INDEX[0])->model.Train(dataset);
        auto root = (HisType *)INDEX[0];
        switch (kLeafNodeID)
        {
            case 0:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new ArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x40000000 + idx);
                }
                break;
            case 1:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new GappedArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x50000000 + idx);
                }
                break;
        }
    }
    break;
    case 3:
    {
        INDEX.push_back((void *)new BSType(childNum));
        ((BSType *)INDEX[0])->model.Train(dataset);
        auto root = (BSType *)INDEX[0];
        switch (kLeafNodeID)
        {
            case 0:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new ArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x40000000 + idx);
                }
                break;
            case 1:
                for(int i=0;i<childNum;i++)
                {
                    INDEX.push_back((void *)new GappedArrayType(kThreshold));
                    int idx = INDEX.size()-1;
                    root->child.push_back(0x50000000 + idx);
                }
                break;
        }
    }
    break;
    }
    auto tmpDataset = dataset;
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    shuffle(tmpDataset.begin(), tmpDataset.end(), default_random_engine(seed));

    cout << "train the next level!" << endl;
    for (int i = 0; i < tmpDataset.size(); i++)
    {
        AdaptiveInsert(tmpDataset[i]);
    }
    cout << "End train" << endl;
}

void ClearAll(int idx, int type)
{
    void * currentNode = INDEX[idx];
    switch (type)
    {
    case 0:
    {
        for(int i=0;i<((LRType *)currentNode)->child.size();i++)
        {
            auto content = ((LRType *)currentNode)->child[i];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            ClearAll(idx, type);
        }
        delete (LRType *)currentNode;
    }
    break;
    case 1:
    {
        for(int i=0;i<((NNType *)currentNode)->child.size();i++)
        {
            auto content = ((NNType *)currentNode)->child[i];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            ClearAll(idx, type);
        }
        delete (NNType *)currentNode;
    }
    break;
    case 2:
    {
        for(int i=0;i<((HisType *)currentNode)->child.size();i++)
        {
            auto content = ((HisType *)currentNode)->child[i];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            ClearAll(idx, type);
        }
        delete (HisType *)currentNode;
    }
    break;
    case 3:
    {
        for(int i=0;i<((BSType *)currentNode)->child.size();i++)
        {
            auto content = ((BSType *)currentNode)->child[i];
            type = content >> 28;
            idx = content & 0x0FFFFFFF;
            ClearAll(idx, type);
        }
        delete (BSType *)currentNode;
    }
    break;
    case 4:
    {
        ((ArrayType *)currentNode)->m_dataset.clear();
        vector<pair<double, double>>().swap(((ArrayType *)currentNode)->m_dataset);
        delete (ArrayType *)currentNode;
    }
    break;
    case 5:
    {
        ((GappedArrayType *)currentNode)->m_dataset.clear();
        vector<pair<double, double>>().swap(((GappedArrayType *)currentNode)->m_dataset);
        delete (GappedArrayType *)currentNode;
    }
    break;
}
}
#endif



