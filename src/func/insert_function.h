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

extern vector<BaseNode> entireChild;

extern LRType lrRoot;
extern NNType nnRoot;
extern HisType hisRoot;
extern BSType bsRoot;

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
            idx = lrRoot.childLeft + lrRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 1:
        {
            idx = nnRoot.childLeft + nnRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 2:
        {
            idx = hisRoot.childLeft + hisRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 3:
        {
            idx = bsRoot.childLeft + bsRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 4:
        {
            idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 5:
        {
            idx = entireChild[idx].nn.childLeft + entireChild[idx].nn.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 6:
        {
            idx = entireChild[idx].his.childLeft + entireChild[idx].his.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 7:
        {
            idx = entireChild[idx].bs.childLeft + entireChild[idx].bs.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 8:
        {
            auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
            if (size >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = entireChild[idx].array.m_left;
                auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = LRModel(); // create a new inner node
                int childNum = 128;
                node.SetChildNumber(128);
                node.childLeft = allocateChildMemory(childNum);
                node.Train(tmpDataset);
                entireChild[idx].lr = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    ArrayType tmpLeaf(kThreshold);
                    tmpLeaf.SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[node.childLeft + i].array = tmpLeaf;
                }
                idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(data.first);
            }
            auto left = entireChild[idx].array.m_left;
            if (size == 0)
            {
                entireData[left] = data;
                entireChild[idx].array.flagNumber++;
                entireChild[idx].array.SetDataset(left, 1, entireChild[idx].array.m_capacity);
                return true;
            }
            int preIdx = entireChild[idx].array.Predict(data.first);
            int start = max(0, preIdx - entireChild[idx].array.error) + left;
            int end = min(size - 1, preIdx + entireChild[idx].array.error) + left;
            start = min(start, end);

            if (data.first <= entireData[start].first)
                preIdx = ArrayBinarySearch(data.first, left, start);
            else if (data.first <= entireData[end].first)
                preIdx = ArrayBinarySearch(data.first, start, end);
            else
                preIdx = ArrayBinarySearch(data.first, end, left + size - 1);

            // expand
            if (size >= entireChild[idx].array.m_capacity)
            {
                auto diff = preIdx - left;
                entireChild[idx].array.SetDataset(left, size, entireChild[idx].array.m_capacity);
                left = entireChild[idx].array.m_left;
                preIdx = left + diff;
            }

            // Insert data
            if ((preIdx == left + size - 1) && (entireData[preIdx].first < data.first))
            {
                entireData[left + size] = data;
                entireChild[idx].array.flagNumber++;
                return true;
            }
            entireChild[idx].array.flagNumber++;
            for (int i = left + size; i > preIdx; i--)
                entireData[i] = entireData[i - 1];
            entireData[preIdx] = data;
            return true;
        }
        break;
        case 9:
        {
            auto left = entireChild[idx].ga.m_left;
            int size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
            if (size >= 4096)
            {
                vector<pair<double, double>> tmpDataset;
                auto left = entireChild[idx].ga.m_left;
                auto size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
                for (int i = left; i < left + size; i++)
                    tmpDataset.push_back(entireData[i]);

                auto node = LRModel(); // create a new inner node
                node.SetChildNumber(128);
                int childNum = 128;
                node.childLeft = allocateChildMemory(childNum);
                node.Train(tmpDataset);
                entireChild[idx].lr = node;

                vector<vector<pair<double, double>>> subFindData;
                vector<pair<double, double>> tmp;
                for (int i = 0; i < childNum; i++)
                    subFindData.push_back(tmp);

                for (int i = 0; i < size; i++)
                {
                    int p = node.Predict(tmpDataset[i].first);
                    subFindData[p].push_back(tmpDataset[i]);
                }

                for (int i = 0; i < childNum; i++)
                {
                    GappedArrayType tmpLeaf(kThreshold);
                    tmpLeaf.SetDataset(subFindData[i], kMaxKeyNum);
                    entireChild[node.childLeft + i].ga = tmpLeaf;
                }
                idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(data.first);
            }
            if ((float(size) / entireChild[idx].ga.capacity > entireChild[idx].ga.density))
            {
                // If an additional Insertion results in crossing the density
                // then we expand the gapped array
                entireChild[idx].ga.SetDataset(left, size, entireChild[idx].ga.capacity);
                left = entireChild[idx].ga.m_left;
            }

            if (size == 0)
            {
                entireData[left] = data;
                entireChild[idx].ga.flagNumber++;
                entireChild[idx].ga.maxIndex = 0;
                entireChild[idx].ga.SetDataset(left, 1, entireChild[idx].ga.capacity);
                return true;
            }
            int preIdx = entireChild[idx].ga.Predict(data.first);

            int start = max(0, preIdx - entireChild[idx].ga.error) + left;
            int end = min(entireChild[idx].ga.maxIndex, preIdx + entireChild[idx].ga.error) + left;
            start = min(start, end);

            if (entireData[start].first == DBL_MIN)
                start--;
            if (entireData[end].first == DBL_MIN)
                end--;

            if (data.first <= entireData[start].first)
                preIdx = GABinarySearch(data.first, left, start);
            else if (data.first <= entireData[end].first)
                preIdx = GABinarySearch(data.first, start, end);
            else
                preIdx = GABinarySearch(data.first, end, left + entireChild[idx].ga.maxIndex);

            // if the Insertion position is a gap,
            //  then we Insert the element into the gap and are done
            if (entireData[preIdx].first == DBL_MIN)
            {
                entireData[preIdx] = data;
                entireChild[idx].ga.flagNumber++;
                entireChild[idx].ga.maxIndex = max(entireChild[idx].ga.maxIndex, preIdx - left);
                return true;
            }
            else
            {
                if (entireData[preIdx].second == DBL_MIN)
                {
                    entireData[preIdx] = data;
                    entireChild[idx].ga.flagNumber++;
                    entireChild[idx].ga.maxIndex = max(entireChild[idx].ga.maxIndex, preIdx - left);
                    return true;
                }
                if (preIdx == left + entireChild[idx].ga.maxIndex && entireData[left + entireChild[idx].ga.maxIndex].first < data.first)
                {
                    entireChild[idx].ga.maxIndex = entireChild[idx].ga.maxIndex + 1;
                    ;
                    entireData[entireChild[idx].ga.maxIndex + left] = data;
                    entireChild[idx].ga.flagNumber++;
                    return true;
                }

                // If the Insertion position is not a gap, we make
                // a gap at the Insertion position by shifting the elements
                // by one position in the direction of the closest gap
                int i = preIdx + 1;
                while (entireData[i].first != DBL_MIN)
                    i++;
                if (i >= left + entireChild[idx].ga.capacity)
                {
                    i = preIdx - 1;
                    while (i >= left && entireData[i].first != DBL_MIN)
                        i--;
                    for (int j = i; j < preIdx - 1; j++)
                        entireData[j] = entireData[j + 1];
                    preIdx--;
                }
                else
                {
                    if (i > entireChild[idx].ga.maxIndex + left)
                        entireChild[idx].ga.maxIndex++;
                    for (; i > preIdx; i--)
                        entireData[i] = entireData[i - 1];
                }
                entireData[preIdx] = data;
                entireChild[idx].ga.flagNumber++;
                entireChild[idx].ga.maxIndex = max(entireChild[idx].ga.maxIndex, preIdx - left);
                return true;
            }
            return false;
        }
        break;
        }
    }
}

#endif // !INSERT_FUNCTION_H