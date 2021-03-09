#ifndef UPDATE_FUNCTION_H
#define UPDATE_FUNCTION_H

#include "inlineFunction.h"
#include "../carmi.h"
using namespace std;

bool CARMI::Update(pair<double, double> data)
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
            idx = root.lrRoot.childLeft + root.lrRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 1:
        {
            idx = root.plrRoot.childLeft + root.plrRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 2:
        {
            idx = root.hisRoot.childLeft + root.hisRoot.model.Predict(data.first);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 3:
        {
            idx = root.bsRoot.childLeft + root.bsRoot.model.Predict(data.first);
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
            idx = entireChild[idx].plr.childLeft + entireChild[idx].plr.Predict(data.first);
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
        case 9:
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
        case 10:
        {
            auto size = entireChild[idx].ycsbLeaf.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].ycsbLeaf.Predict(data.first);
            auto left = entireChild[idx].ycsbLeaf.m_left;
            if (entireData[left + preIdx].first == data.first)
            {
                entireData[left + preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].ycsbLeaf.error) + left;
                int end = min(size - 1, preIdx + entireChild[idx].ycsbLeaf.error) + left;
                start = min(start, end);
                int res;
                if (data.first <= entireData[start].first)
                    res = YCSBBinarySearch(data.first, left, start);
                else if (data.first <= entireData[end].first)
                    res = YCSBBinarySearch(data.first, start, end);
                else
                {
                    res = YCSBBinarySearch(data.first, end, left + size - 1);
                    if (res >= left + size)
                        return false;
                }
                if (entireData[res].first == data.first)
                    entireData[res].second = data.second;
                return true;
                return false;
            }
        }
        break;
        }
    }
}

#endif // !UPDATE_FUNCTION_H