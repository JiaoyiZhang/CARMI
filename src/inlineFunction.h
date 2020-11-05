#ifndef INLINE_FUNCTION_H
#define INLINE_FUNCTION_H

#include "params.h"
#include "innerNodeType/bin_type.h"
#include "innerNodeType/his_type.h"
#include "innerNodeType/lr_type.h"
#include "innerNodeType/nn_type.h"
#include "leafNodeType/ga_type.h"
#include "leafNodeType/array_type.h"
#include <vector>
using namespace std;

// search a key-value through binary search in 
// the array leaf node
inline int ArrayBinarySearch(vector<pair<double, double>> &m_dataset, double key, int start, int end)
{
    while (start < end)
    {
        int mid = (start + end) / 2;
        if (m_dataset[mid].first < key)
            start = mid + 1;
        else
            end = mid;
    }
    return start;
}

// search a key-value through binary search 
// in the gapped array
inline int GABinarySearch(vector<pair<double, double>> &m_dataset, double key, int start_idx, int end_idx)
{
    // use binary search to find
    while (start_idx < end_idx)
    {
        if (m_dataset[start_idx].first == -1)
            start_idx--;
        if (m_dataset[end_idx].first == -1)
            end_idx++;
        int mid = (start_idx + end_idx) >> 1;
        if (m_dataset[mid].first == -1)
        {
            int left = max(start_idx, mid - 1);
            int right = min(end_idx, mid + 1);
            if (m_dataset[left].first >= key)
                end_idx = left;
            else
                start_idx = right;
        }
        else
        {
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    return start_idx;
}

#endif