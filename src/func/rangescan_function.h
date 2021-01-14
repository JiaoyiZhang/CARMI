#ifndef RANGE_SCAN_FUNCTION_H
#define RANGE_SCAN_FUNCTION_H

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

vector<pair<double, double>> GetValues(int idx, int length)
{
    vector<pair<double, double>> values;
    if ((entireChild[idx].array.flagNumber >> 24) == 8)
    {
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        auto left = entireChild[idx].array.m_left;
        if (length > size)
        {
            int end = left + size;
            for (int i = left; i < end; i++)
            {
                values.push_back(entireData[i]);
            }
            length = length - size;
            auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
            values.insert(values.end(), otherValues.begin(), otherValues.end());
        }
        else
        {
            int end = left + length;
            for (int i = left; i < end; i++)
            {
                values.push_back(entireData[i]);
            }
        }
    }
    else
    {
        auto left = entireChild[idx].ga.m_left;
        auto size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
        int end = left + entireChild[idx].ga.maxIndex;
        for (int i = left; i < end; i++)
        {
            if (entireData[i].first != DBL_MIN)
            {
                values.push_back(entireData[i]);
                length--;
            }
            if (length == 0)
                break;
        }
        if (length > 0)
        {
            auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
            values.insert(values.end(), otherValues.begin(), otherValues.end());
        }
    }

    return values;
}

vector<pair<double, double>> RangeScan(int rootType, double key, int length)
{
    int idx = 0; // idx in the INDEX
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            idx = lrRoot.childLeft + lrRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 1:
        {
            idx = nnRoot.childLeft + nnRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 2:
        {
            idx = hisRoot.childLeft + hisRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 3:
        {
            idx = bsRoot.childLeft + bsRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 4:
        {
            idx = entireChild[idx].lr.childLeft + entireChild[idx].lr.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 5:
        {
            idx = entireChild[idx].nn.childLeft + entireChild[idx].nn.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 6:
        {
            idx = entireChild[idx].his.childLeft + entireChild[idx].his.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 7:
        {
            idx = entireChild[idx].bs.childLeft + entireChild[idx].bs.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 8:
        {
            vector<pair<double, double>> values;
            auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].array.Predict(key);
            auto left = entireChild[idx].array.m_left;
            if (entireData[left + preIdx].first == key)
            {
                if (length > size - preIdx + 1)
                {
                    int end = left + size;
                    for (int i = left + preIdx; i < end; i++)
                    {
                        values.push_back(entireData[i]);
                    }
                    length = length - (size - preIdx);
                    auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
                    values.insert(values.end(), otherValues.begin(), otherValues.end());
                }
                else
                {
                    int end = left + preIdx + length;
                    for (int i = left + preIdx; i < end; i++)
                    {
                        values.push_back(entireData[i]);
                    }
                }
                return values;
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].array.error) + left;
                int end = min(size - 1, preIdx + entireChild[idx].array.error) + left;
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
                {
                    if (length > size - res)
                    {
                        int end = left + size;
                        for (int i = left + res; i < end; i++)
                        {
                            values.push_back(entireData[i]);
                        }
                        length = length - (size - res);
                        auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
                        values.insert(values.end(), otherValues.begin(), otherValues.end());
                    }
                    else
                    {
                        int end = left + res + length;
                        for (int i = left + res; i < end; i++)
                        {
                            values.push_back(entireData[i]);
                        }
                    }
                    return values;
                }
                return {};
            }
        }
        break;
        case 9:
        {
            vector<pair<double, double>> values;
            auto left = entireChild[idx].ga.m_left;
            int preIdx = entireChild[idx].ga.Predict(key);
            auto size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
            if (entireData[left + preIdx].first == key)
            {
                int end = left + entireChild[idx].ga.maxIndex;
                for (int i = left + preIdx; i < end; i++)
                {
                    if (entireData[i].first != DBL_MIN)
                    {
                        values.push_back(entireData[i]);
                        length--;
                    }
                    if (length == 0)
                        break;
                }
                if (length > 0)
                {
                    auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
                    values.insert(values.end(), otherValues.begin(), otherValues.end());
                }
                return values;
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].ga.error) + left;
                int end = min(entireChild[idx].ga.maxIndex, preIdx + entireChild[idx].ga.error) + left;
                start = min(start, end);

                int res;
                if (entireData[start].first == DBL_MIN)
                    start--;
                if (entireData[end].first == DBL_MIN)
                    end--;
                if (key <= entireData[start].first)
                    res = GABinarySearch(key, left, start);
                else if (key <= entireData[end].first)
                    res = GABinarySearch(key, start, end);
                else
                {
                    res = GABinarySearch(key, end, left + entireChild[idx].ga.maxIndex);
                    if (res > left + entireChild[idx].ga.maxIndex)
                        return {};
                }

                if (entireData[res].first == key)
                {
                    int end = left + entireChild[idx].ga.maxIndex;
                    for (int i = left + res; i < end; i++)
                    {
                        if (entireData[i].first != DBL_MIN)
                        {
                            values.push_back(entireData[i]);
                            length--;
                        }
                        if (length == 0)
                            break;
                    }
                    if (length > 0)
                    {
                        auto otherValues = GetValues(entireChild[idx].array.nextLeaf, length);
                        values.insert(values.end(), otherValues.begin(), otherValues.end());
                    }
                    return values;
                }
                return {};
            }
        }
        break;
        }
    }
}

#endif // !RANGE_SCAN_FUNCTION_H