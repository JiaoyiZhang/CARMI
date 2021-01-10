#ifndef INLINE_FUNCTION_H
#define INLINE_FUNCTION_H

#include "../params.h"
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
#include <vector>
using namespace std;

extern vector<BaseNode> entireChild;

extern pair<double, double> *entireData;
extern vector<pair<double, double>> findDatapoint;

// search a key-value through binary search in
// the array leaf node
inline int ArrayBinarySearch(double key, int start, int end)
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
inline int GABinarySearch(double key, int start_idx, int end_idx)
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

// designed for construction

inline int TestArrayBinarySearch(double key, int start, int end)
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (findDatapoint[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

inline int TestGABinarySearch(double key, int start_idx, int end_idx)
{
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        if (findDatapoint[mid].first == -1)
        {
            if (findDatapoint[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            if (findDatapoint[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    if (findDatapoint[start_idx].first >= key)
        return start_idx;
    else
        return end_idx;
}

#endif