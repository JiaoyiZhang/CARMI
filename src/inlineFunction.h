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
// return the idx of the first element >= key
inline int GABinarySearch(vector<pair<double, double>> &m_dataset, double key, int start_idx, int end_idx)
{
    // for(int i=0;i<m_dataset.size();i++)
    // {
    //     cout<<i<<":"<<m_dataset[i].first<<"\t";
    //     if((i+1)%100 == 0)
    //         cout<<endl;
    // }
    // cout<<endl;
    // use binary search to find
    // cout<<"key:"<<key<<"\tstart:"<<start_idx<<"\tend:"<<end_idx<<endl;
    while (end_idx - start_idx >= 2)
    {
        int mid = (start_idx + end_idx) >> 1;
        // cout<<"In while: mid:"<<mid<<"\tstart:"<<start_idx<<"\tend:"<<end_idx<<endl;
        if (m_dataset[mid].first == -1)
        {
            // cout<<"mid == -1!\tm_dataset[mid - 1].first:"<<m_dataset[mid - 1].first<<"\tkey:"<<key<<endl;
            if (m_dataset[mid - 1].first >= key)
                end_idx = mid - 1;
            else
                start_idx = mid + 1;
        }
        else
        {
            // cout<<"mid != -1!\tm_dataset[mid].first:"<<m_dataset[mid].first<<"\tkey:"<<key<<endl;
            if (m_dataset[mid].first >= key)
                end_idx = mid;
            else
                start_idx = mid + 1;
        }
    }
    // cout<<"Out of while! start:"<<start_idx<<"\tm_dataset[start_idx].first:"<<m_dataset[start_idx].first<<"\tm_dataset[end_idx].first:"<<m_dataset[end_idx].first<<endl;
    if (m_dataset[start_idx].first >= key)
        return start_idx;
    else
        return end_idx;
}

#endif