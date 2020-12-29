#ifndef INSERT_FUNCTION_H
#define INSERT_FUNCTION_H
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"

#include "../innerNode/bs_model.h"
#include "../innerNode/lr_model.h"
#include "../innerNode/nn_model.h"
#include "../innerNode/his_model.h"

#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"

#include "inlineFunction.h"
#include "../dataManager/datapoint.h"
using namespace std;

extern BaseNode **entireChild;

bool Insert(int rootType, pair<double, double> data)
{
    int idx = 0; // idx in the INDEX
    int content;
    int type = rootType;
    int childIdx = 0;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            idx = ((LRType *)entireChild[idx])->childLeft + ((LRType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;

            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new LRModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 65;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new LRModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 65;
            }
        }
        break;
        case 1:
        {
            idx = ((NNType *)entireChild[idx])->childLeft + ((NNType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;

            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new NNModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 66;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new NNModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 66;
            }
        }
        break;
        case 2:
        {
            idx = ((HisType *)entireChild[idx])->childLeft + ((HisType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;

            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new HisModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 67;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new HisModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 67;
            }
        }
        break;
        case 3:
        {
            idx = ((BSType *)entireChild[idx])->childLeft + ((BSType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;

            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new BSModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 68;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new BSModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 68;
            }
        }
        break;
        case 65:
        {
            idx = ((LRModel *)entireChild[idx])->childLeft + ((LRModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new LRModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 65;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new LRModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 65;
            }
        }
        break;
        case 66:
        {
            idx = ((NNModel *)entireChild[idx])->childLeft + ((NNModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new NNModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 66;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new NNModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 66;
            }
        }
        break;
        case 67:
        {
            idx = ((HisModel *)entireChild[idx])->childLeft + ((HisModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new HisModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 67;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new HisModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 67;
            }
        }
        break;
        case 68:
        {
            idx = ((BSModel *)entireChild[idx])->childLeft + ((BSModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
            // check split
            if (type == 4 && ((ArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((ArrayType *)entireChild[idx])->m_left;
                auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new BSModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new ArrayType(kThreshold);
                    ((ArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 68;
            }
            else if (type == 5 && ((GappedArrayType *)entireChild[idx])->m_datasetSize >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = ((GappedArrayType *)entireChild[idx])->m_left;
                auto size = ((GappedArrayType *)entireChild[idx])->m_datasetSize;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = new BSModel(128); // create a new inner node
                int childNum = node->childNumber;
                node->childLeft = allocateChildMemory(childNum);
                node->Train(tmpDataset);
                entireChild[idx] = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node->Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    entireChild[node->childLeft + i] = new GappedArrayType(kThreshold);
                    ((GappedArrayType *)entireChild[node->childLeft + i])->SetDataset(subFindData[i], kMaxKeyNum);
                }
                type = 68;
            }
        }
        break;
        case 69:
        {
            auto left = ((ArrayType *)entireChild[idx])->m_left;
            if (((ArrayType *)entireChild[idx])->m_datasetSize == 0)
            {
                entireData[left] = data;
                ((ArrayType *)entireChild[idx])->m_datasetSize++;
                ((ArrayType *)entireChild[idx])->SetDataset(left, 1, ((ArrayType *)entireChild[idx])->m_capacity);
                return true;
            }
            auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
            int preIdx = ((ArrayType *)entireChild[idx])->model.PredictPrecision(data.first, size);
            int start = max(0, preIdx - ((ArrayType *)entireChild[idx])->error) + left;
            int end = min(size - 1, preIdx + ((ArrayType *)entireChild[idx])->error) + left;
            start = min(start, end);

            if (data.first <= entireData[start].first)
                preIdx = ArrayBinarySearch(data.first, left, start);
            else if (data.first <= entireData[end].first)
                preIdx = ArrayBinarySearch(data.first, start, end);
            else
                preIdx = ArrayBinarySearch(data.first, end, left + size - 1);

            // expand
            if (size >= ((ArrayType *)entireChild[idx])->m_capacity)
            {
                auto diff = preIdx - left;
                ((ArrayType *)entireChild[idx])->SetDataset(left, size, ((ArrayType *)entireChild[idx])->m_capacity);
                left = ((ArrayType *)entireChild[idx])->m_left;
                preIdx = left + diff;
            }

            // Insert data
            if ((preIdx == left + size - 1) && (entireData[preIdx].first < data.first))
            {
                entireData[left + size] = data;
                ((ArrayType *)entireChild[idx])->m_datasetSize++;
                return true;
            }
            ((ArrayType *)entireChild[idx])->m_datasetSize++;
            for (int i = left + size; i > preIdx; i--)
                entireData[i] = entireData[i - 1];
            entireData[preIdx] = data;
            return true;
        }
        break;
        case 70:
        {
            auto left = ((GappedArrayType *)entireChild[idx])->m_left;
            if ((float(((GappedArrayType *)entireChild[idx])->m_datasetSize) / ((GappedArrayType *)entireChild[idx])->capacity > ((GappedArrayType *)entireChild[idx])->density))
            {
                // If an additional Insertion results in crossing the density
                // then we expand the gapped array
                ((GappedArrayType *)entireChild[idx])->SetDataset(left, ((GappedArrayType *)entireChild[idx])->m_datasetSize, ((GappedArrayType *)entireChild[idx])->capacity);
                left = ((GappedArrayType *)entireChild[idx])->m_left;
            }

            if (((GappedArrayType *)entireChild[idx])->m_datasetSize == 0)
            {
                entireData[left] = data;
                ((GappedArrayType *)entireChild[idx])->m_datasetSize++;
                ((GappedArrayType *)entireChild[idx])->maxIndex = 0;
                ((GappedArrayType *)entireChild[idx])->SetDataset(left, 1, ((GappedArrayType *)entireChild[idx])->capacity);
                return true;
            }
            int preIdx = ((GappedArrayType *)entireChild[idx])->model.PredictPrecision(data.first, ((GappedArrayType *)entireChild[idx])->maxIndex + 1);

            int start = max(0, preIdx - ((GappedArrayType *)entireChild[idx])->error) + left;
            int end = min(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx + ((GappedArrayType *)entireChild[idx])->error) + left;
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
                preIdx = GABinarySearch(data.first, end, left + ((GappedArrayType *)entireChild[idx])->maxIndex);

            // if the Insertion position is a gap,
            //  then we Insert the element into the gap and are done
            if (entireData[preIdx].first == -1)
            {
                entireData[preIdx] = data;
                ((GappedArrayType *)entireChild[idx])->m_datasetSize++;
                ((GappedArrayType *)entireChild[idx])->maxIndex = max(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx - left);
                return true;
            }
            else
            {
                if (entireData[preIdx].second == DBL_MIN)
                {
                    entireData[preIdx] = data;
                    ((GappedArrayType *)entireChild[idx])->m_datasetSize++;
                    ((GappedArrayType *)entireChild[idx])->maxIndex = max(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx - left);
                    return true;
                }
                if (preIdx == left + ((GappedArrayType *)entireChild[idx])->maxIndex && entireData[left + ((GappedArrayType *)entireChild[idx])->maxIndex].first < data.first)
                {
                    ((GappedArrayType *)entireChild[idx])->maxIndex = ((GappedArrayType *)entireChild[idx])->maxIndex + 1;
                    ;
                    entireData[((GappedArrayType *)entireChild[idx])->maxIndex + left] = data;
                    ((GappedArrayType *)entireChild[idx])->m_datasetSize++;
                    return true;
                }

                // If the Insertion position is not a gap, we make
                // a gap at the Insertion position by shifting the elements
                // by one position in the direction of the closest gap
                int i = preIdx + 1;
                while (entireData[i].first != -1)
                    i++;
                if (i >= left + ((GappedArrayType *)entireChild[idx])->capacity)
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
                    if (i > ((GappedArrayType *)entireChild[idx])->maxIndex + left)
                        ((GappedArrayType *)entireChild[idx])->maxIndex++;
                    for (; i > preIdx; i--)
                        entireData[i] = entireData[i - 1];
                }
                entireData[preIdx] = data;
                ((GappedArrayType *)entireChild[idx])->m_datasetSize++;
                ((GappedArrayType *)entireChild[idx])->maxIndex = max(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx - left);
                return true;
            }
            return false;
        }
        break;
        }
    }
}

#endif // !INSERT_FUNCTION_H