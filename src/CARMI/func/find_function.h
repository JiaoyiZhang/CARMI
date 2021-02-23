#ifndef FIND_FUNCTION_H
#define FIND_FUNCTION_H

#include "../carmi.h"
#include <float.h>
using namespace std;

CARMI::iterator CARMI::Find(double key)
// pair<double, double> CARMI::Find(double key) const
{
    int idx = 0; // idx in the INDEX
    int type = rootType;
    while (1)
    {
        switch (type)
        {
        case 0:
        {
            idx = root.lrRoot.childLeft + root.lrRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 1:
        {
            idx = root.plrRoot.childLeft + root.plrRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 2:
        {
            idx = root.hisRoot.childLeft + root.hisRoot.model.Predict(key);
            type = entireChild[idx].lr.flagNumber >> 24;
        }
        break;
        case 3:
        {
            idx = root.bsRoot.childLeft + root.bsRoot.model.Predict(key);
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
            idx = entireChild[idx].plr.childLeft + entireChild[idx].plr.Predict(key);
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
            auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].array.Predict(key);
            auto left = entireChild[idx].array.m_left;
            if (entireData[left + preIdx].first == key)
            {
                CARMI::iterator it(this, &entireChild[idx], preIdx);
                return it;
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
                    CARMI::iterator it(this, &entireChild[idx], res - left);
                    return it;
                }
                return {};
            }
        }
        break;
        case 9:
        {
            auto left = entireChild[idx].ga.m_left;
            int preIdx = entireChild[idx].ga.Predict(key);
            if (entireData[left + preIdx].first == key)
            {
                CARMI::iterator it(this, &entireChild[idx], preIdx);
                return it;
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
                    {
                        CARMI::iterator it;
                        return it.end();
                    }
                }

                if (entireData[res].first == key)
                {
                    CARMI::iterator it(this, &entireChild[idx], res - left);
                    return it;
                }
                CARMI::iterator it;
                return it.end();
            }
        }
        break;
        case 10:
        {
            auto size = entireChild[idx].ycsbLeaf.flagNumber & 0x00FFFFFF;
            int preIdx = entireChild[idx].ycsbLeaf.Predict(key);
            auto left = entireChild[idx].ycsbLeaf.m_left;
            if (initDataset[left + preIdx].first == key)
            {
                CARMI::iterator it(this, &entireChild[idx], preIdx);
                return it;
                // return initDataset[left + preIdx];
            }
            else
            {
                int start = max(0, preIdx - entireChild[idx].ycsbLeaf.error) + left;
                int end = min(size - 1, preIdx + entireChild[idx].ycsbLeaf.error) + left;
                start = min(start, end);
                int res;
                if (key <= initDataset[start].first)
                    res = YCSBBinarySearch(key, left, start);
                else if (key <= initDataset[end].first)
                    res = YCSBBinarySearch(key, start, end);
                else
                {
                    res = YCSBBinarySearch(key, end, left + size - 1);
                    if (res >= left + size)
                        return {};
                }
                if (initDataset[res].first == key)
                {
                    CARMI::iterator it(this, &entireChild[idx], res - left);
                    return it;
                    // return initDataset[res];
                }
                return {};
            }
        }
        break;
        }
    }
}

#endif // !FIND_FUNCTION_H