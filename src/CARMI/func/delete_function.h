#ifndef DELETE_FUNCTION_H
#define DELETE_FUNCTION_H
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
        case 69:
        {
            auto left = entireChild[idx].array.m_left;
            auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].array.Predict(key);
            if (entireData[left + preIdx].first == key)
                preIdx += left;
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
            entireChild[idx].array.flagNumber--;
            return true;
        }
        break;
        case 70:
        {
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            auto left = entireChild[idx].ga.m_left;
            int preIdx = entireChild[idx].ga.Predict(key);
            if (entireData[left + preIdx].first == key)
            {
                entireData[left + preIdx].second = DBL_MIN;
                entireChild[idx].ga.flagNumber--;
                if (preIdx == entireChild[idx].ga.maxIndex)
                    entireChild[idx].ga.maxIndex--;
                return true;
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].ga.error) + left;
                int end = min(entireChild[idx].ga.maxIndex, preIdx + entireChild[idx].ga.error) + left;
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
                    res = GABinarySearch(key, end, left + entireChild[idx].ga.maxIndex);
                    if (res > left + entireChild[idx].ga.maxIndex)
                        return false;
                }

                if (entireData[res].first != key)
                    return false;
                entireChild[idx].ga.flagNumber--;
                entireData[res] = {DBL_MIN, DBL_MIN};
                if (res == left + entireChild[idx].ga.maxIndex)
                    entireChild[idx].ga.maxIndex--;
                return true;
            }
        }
        break;
        }
    }
}

#endif // !DELETE_FUNCTION_H