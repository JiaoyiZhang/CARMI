/**
 * @file insert_function.h
 * @author Jiaoyi
 * @brief insert a record
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
#define SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "../construct/minor_function.h"

template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Insert(DataType data) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  int childIdx = 0;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft +
              root.LRType<DataVectorType, DataType>::model.Predict(data.first);
        break;
      case LR_INNER_NODE:
        idx = entireChild[idx].lr.childLeft +
              entireChild[idx].lr.Predict(data.first);
        break;
      case PLR_INNER_NODE:
        idx = entireChild[idx].plr.childLeft +
              entireChild[idx].plr.Predict(data.first);
        break;
      case HIS_INNER_NODE:
        idx = entireChild[idx].his.childLeft +
              entireChild[idx].his.Predict(data.first);
        break;
      case BS_INNER_NODE:
        idx = entireChild[idx].bs.childLeft +
              entireChild[idx].bs.Predict(data.first);
        break;
      case ARRAY_LEAF_NODE: {
        int size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        int left = entireChild[idx].array.m_left;

        // split
        if (size >= carmi_params::kLeafMaxCapacity) {
          int previousIdx = entireChild[idx].array.previousLeaf;
          Split<ArrayType>(false, left, size, previousIdx, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
          left = entireChild[idx].array.m_left;
          size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        }

        // size == 0
        if (size == 0) {
          entireData[left] = data;
          entireChild[idx].array.flagNumber++;
          Init(entireChild[idx].array.m_capacity, left, 1, entireData,
               &entireChild[idx].array);
          return true;
        }
        int preIdx = entireChild[idx].array.Predict(data.first);
        preIdx = ArraySearch(data.first, preIdx, entireChild[idx].array.error,
                             left, size);

        // expand
        if ((size >= entireChild[idx].array.m_capacity) &&
            entireChild[idx].array.m_capacity <
                carmi_params::kLeafMaxCapacity) {
          auto diff = preIdx - left;
          Init(entireChild[idx].array.m_capacity, left, size, entireData,
               &entireChild[idx].array);
          left = entireChild[idx].array.m_left;
          preIdx = left + diff;
        }

        // Insert data
        if ((preIdx == left + size - 1) &&
            (entireData[preIdx].first < data.first)) {
          entireData[left + size] = data;
          entireChild[idx].array.flagNumber++;
          return true;
        }
        entireChild[idx].array.flagNumber++;
        for (int i = left + size; i > preIdx; i--)
          entireData[i] = entireData[i - 1];
        entireData[preIdx] = data;
        return true;
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        int left = entireChild[idx].externalArray.m_left;
        int size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;

        // split
        if (size >= carmi_params::kLeafMaxCapacity) {
          Split<ExternalArray>(true, left, size, 0, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
          left = entireChild[idx].externalArray.m_left;
          size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        }
        if (size > 0) {
          entireChild[idx].externalArray.flagNumber++;
        } else if (size == 0) {
          entireChild[idx].externalArray.m_left = curr;
          DataVectorType trainData;
          trainData.push_back(data);
          Train(0, 1, trainData, &entireChild[idx].externalArray);
        }

        entireChild[idx].externalArray.flagNumber++;
        curr++;

        return true;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
