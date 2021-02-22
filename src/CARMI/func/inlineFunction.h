#ifndef INLINE_FUNCTION_H
#define INLINE_FUNCTION_H

#include "../../params.h"
#include <vector>
#include <float.h>
#include "../carmi.h"
using namespace std;

inline bool CARMI::scan(const int left, const int end, vector<pair<double, double>> &ret, int &firstIdx, int &length) const
{
    for (int i = left; i < end; i++)
    {
        if (length == 0)
            return true;
        if (entireData[i].first != DBL_MIN)
        {
            ret[firstIdx] = entireData[i];
            firstIdx++;
            length--;
        }
    }
    if (length != 0)
        return false;
    else
        return true;
}

// search a key-value through binary search in
// the array leaf node
inline int CARMI::ArrayBinarySearch(double key, int start, int end) const
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (entireData[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

// search a key-value through binary search
// in the gapped array
// return the idx of the first element >= key
inline int CARMI::GABinarySearch(double key, int start_idx, int end_idx) const
{
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        if (entireData[mid].first == DBL_MIN)
        {
            if (entireData[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            if (entireData[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    if (entireData[start_idx].first >= key)
        return start_idx;
    else
        return end_idx;
}

// search a key-value through binary search in
// the YCSB leaf node
inline int CARMI::YCSBBinarySearch(double key, int start, int end) const
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (initDataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

// designed for construction
inline int CARMI::TestArrayBinarySearch(double key, int start, int end) const
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (initDataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

inline int CARMI::TestGABinarySearch(double key, int start_idx, int end_idx) const
{
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        if (initDataset[mid].first == -1)
        {
            if (initDataset[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            if (initDataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    if (initDataset[start_idx].first >= key)
        return start_idx;
    else
        return end_idx;
}

#endif