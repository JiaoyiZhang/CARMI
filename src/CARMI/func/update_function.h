#ifndef UPDATE_FUNCTION_H
#define UPDATE_FUNCTION_H

#include "inlineFunction.h"
#include "../dataManager/datapoint.h"
using namespace std;

extern vector<BaseNode> entireChild;

extern LRType lrRoot;
extern PLRType plrRoot;
extern HisType hisRoot;
extern BSType bsRoot;

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
            idx = lrRoot.childLeft + lrRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 1:
        {
            idx = plrRoot.childLeft + plrRoot.model.Predict(data.first);
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
        case 69:
        {
            auto left = entireChild[idx].array.m_left;
            auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].array.Predict(data.first);
            if (entireData[left + preIdx].first == data.first)
                entireData[left + preIdx].second = data.second;
            else
            {
                int start = max(0, preIdx - entireChild[idx].array.error) + left;
                int end = min(size - 1, preIdx + entireChild[idx].array.error) + left;
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
        case 70:
        {
            auto left = entireChild[idx].ga.m_left;
            int preIdx = entireChild[idx].ga.Predict(data.first);
            if (entireData[left + preIdx].first == data.first)
            {
                entireData[left + preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].ga.error) + left;
                int end = min(entireChild[idx].ga.maxIndex, preIdx + entireChild[idx].ga.error) + left;
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
                    preIdx = GABinarySearch(data.first, end, left + entireChild[idx].ga.maxIndex);
                    if (preIdx > left + entireChild[idx].ga.maxIndex)
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

#endif // !UPDATE_FUNCTION_H