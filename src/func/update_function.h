#ifndef UPDATE_FUNCTION_H
#define UPDATE_FUNCTION_H
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
            idx = ((LRType *)entireChild[idx])->childLeft + ((LRType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 1:
        {
            idx = ((NNType *)entireChild[idx])->childLeft + ((NNType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 2:
        {
            idx = ((HisType *)entireChild[idx])->childLeft + ((HisType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 3:
        {
            idx = ((BSType *)entireChild[idx])->childLeft + ((BSType *)entireChild[idx])->model.Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 65:
        {
            idx = ((LRModel *)entireChild[idx])->childLeft + ((LRModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 66:
        {
            idx = ((NNModel *)entireChild[idx])->childLeft + ((NNModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 67:
        {
            idx = ((HisModel *)entireChild[idx])->childLeft + ((HisModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 68:
        {
            idx = ((BSModel *)entireChild[idx])->childLeft + ((BSModel *)entireChild[idx])->Predict(data.first);
            type = entireChild[idx]->flag;
        }
        break;
        case 69:
        {
            auto left = ((ArrayType *)entireChild[idx])->m_left;
            auto size = ((ArrayType *)entireChild[idx])->m_datasetSize;
            int preIdx = ((ArrayType *)entireChild[idx])->model.PredictPrecision(data.first, size);
            if (entireData[left + preIdx].first == data.first)
                entireData[left + preIdx].second = data.second;
            else
            {
                int start = max(0, preIdx - ((ArrayType *)entireChild[idx])->error) + left;
                int end = min(size - 1, preIdx + ((ArrayType *)entireChild[idx])->error) + left;
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
            auto left = ((GappedArrayType *)entireChild[idx])->m_left;
            int preIdx = ((GappedArrayType *)entireChild[idx])->model.PredictPrecision(data.first, ((GappedArrayType *)entireChild[idx])->maxIndex + 1);
            if (entireData[left + preIdx].first == data.first)
            {
                entireData[left + preIdx].second = data.second;
                return true;
            }
            else
            {
                int start = max(0, preIdx - ((GappedArrayType *)entireChild[idx])->error) + left;
                int end = min(((GappedArrayType *)entireChild[idx])->maxIndex, preIdx + ((GappedArrayType *)entireChild[idx])->error) + left;
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
                    preIdx = GABinarySearch(data.first, end, left + ((GappedArrayType *)entireChild[idx])->maxIndex);
                    if (preIdx > left + ((GappedArrayType *)entireChild[idx])->maxIndex)
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