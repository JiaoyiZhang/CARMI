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

extern BaseNode **entireChild;

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
            idx = ((LRType *)entireChild[idx])->childLeft + ((LRType *)entireChild[idx])->model.Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 1:
        {
            idx = ((NNType *)entireChild[idx])->childLeft + ((NNType *)entireChild[idx])->model.Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 2:
        {
            idx = ((HisType *)entireChild[idx])->childLeft + ((HisType *)entireChild[idx])->model.Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 3:
        {
            idx = ((BSType *)entireChild[idx])->childLeft + ((BSType *)entireChild[idx])->model.Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 65:
        {
            idx = ((LRModel *)entireChild[idx])->childLeft + ((LRModel *)entireChild[idx])->Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 66:
        {
            idx = ((NNModel *)entireChild[idx])->childLeft + ((NNModel *)entireChild[idx])->Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 67:
        {
            idx = ((HisModel *)entireChild[idx])->childLeft + ((HisModel *)entireChild[idx])->Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 68:
        {
            idx = ((BSModel *)entireChild[idx])->childLeft + ((BSModel *)entireChild[idx])->Predict(key);
            type = entireChild[idx]->flag;
        }
        break;
        case 69:
        {
            auto left = ((ArrayType *)entireChild[idx])->m_left;
            auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
            int preIdx = ((ArrayType *)entireChild[idx])->model.PredictPrecision(key, size);
            if (entireData[left + preIdx].first == key)
                preIdx += left;
            else
            {
                int start = max(0, preIdx - ((ArrayType *)entireChild[idx])->error) + left;
                int end = min(size - 1, preIdx + ((ArrayType *)entireChild[idx])->error) + left;
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
            ((ArrayType *)entireChild[idx])->m_datasetSize--;
            return true;
        }
        break;
        case 70:
        {
            // DBL_MIN means the data has been deleted
            // when a data has been deleted, data.second == DBL_MIN
            auto left = ((GappedArrayType *)entireChild[idx])->m_left;
            int preIdx = ((GappedArrayType *)entireChild[idx])->model.PredictPrecision(key, ((GappedArrayType *)entireChild[idx])->maxIndex + 1);
            if (entireData[left + preIdx].first == key)
            {
                entireData[left + preIdx].second = DBL_MIN;
                ((GappedArrayType *)entireChild[idx])->m_datasetSize--;
                if (preIdx == ((GappedArrayType *)entireChild[idx])->maxIndex)
                    ((GappedArrayType *)entireChild[idx])->maxIndex--;
                return true;
            }
            else
            {
                int start = max(0, preIdx - ((GappedArrayType *)entireChild[idx])->error) + left;
                int end = min(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx + ((GappedArrayType *)entireChild[idx])->error) + left;
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
                    res = GABinarySearch(key, end, left + ((GappedArrayType *)entireChild[idx])->maxIndex);
                    if (res > left + ((GappedArrayType *)entireChild[idx])->maxIndex)
                        return false;
                }

                if (entireData[res].first != key)
                    return false;
                ((GappedArrayType *)entireChild[idx])->m_datasetSize--;
                entireData[res] = {DBL_MIN, DBL_MIN};
                if (res == left + ((GappedArrayType *)entireChild[idx])->maxIndex)
                    ((GappedArrayType *)entireChild[idx])->maxIndex--;
                return true;
            }
        }
        break;
        }
    }
}

#endif // !DELETE_FUNCTION_H