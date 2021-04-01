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

/**
 * @brief insert a data point
 *
 * @param data
 * @return true if the insertion is successful
 * @return false if the operation fails
 */
template <typename KeyType, typename ValueType>
bool CARMI<KeyType, ValueType>::Insert(DataType data) {
  int idx = 0;  // idx in the INDEX
  int type = rootType;
  int childIdx = 0;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.childLeft + root.LRType::model.Predict(data.first);
        break;
      case PLR_ROOT_NODE:
        idx = root.childLeft + root.PLRType::model.Predict(data.first);
        break;
      case HIS_ROOT_NODE:
        idx = root.childLeft + root.HisType::model.Predict(data.first);
        break;
      case BS_ROOT_NODE:
        idx = root.childLeft + root.BSType::model.Predict(data.first);
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
      case GAPPED_ARRAY_LEAF_NODE: {
        int left = entireChild[idx].ga.m_left;
        int size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;

        // split
        if (size >= carmi_params::kLeafMaxCapacity) {
          int previousIdx = entireChild[idx].ga.previousLeaf;
          Split<GappedArrayType>(false, left, entireChild[idx].ga.maxIndex + 1,
                                 previousIdx, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
          left = entireChild[idx].ga.m_left;
          size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
        }

        // expand
        if (entireChild[idx].ga.capacity < carmi_params::kLeafMaxCapacity &&
            (static_cast<float>(size) / entireChild[idx].ga.capacity >
             entireChild[idx].ga.density)) {
          // If an additional Insertion results in crossing the density
          // then we expand the gapped array
          Init(entireChild[idx].ga.capacity, left,
               entireChild[idx].ga.maxIndex + 1, entireData,
               &entireChild[idx].ga);
          left = entireChild[idx].ga.m_left;
          size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
        }

        // size == 0
        if (size == 0) {
          entireData[left] = data;
          entireChild[idx].ga.flagNumber++;
          entireChild[idx].ga.maxIndex = 0;
          Init(entireChild[idx].ga.capacity, left, 1, entireData,
               &entireChild[idx].ga);
          left = entireChild[idx].ga.m_left;
          size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
          return true;
        }

        // find position
        int preIdx = entireChild[idx].ga.Predict(data.first);
        preIdx =
            GASearch(data.first, preIdx, entireChild[idx].externalArray.error,
                     left, entireChild[idx].ga.maxIndex);

        // if the Insertion position is a gap,
        //  then we Insert the element into the gap and are done
        if (entireData[preIdx].first == DBL_MIN) {
          entireData[preIdx] = data;
          entireChild[idx].ga.flagNumber++;
          entireChild[idx].ga.maxIndex =
              std::max(entireChild[idx].ga.maxIndex, preIdx - left);
          return true;
        } else {
          if (preIdx == left + entireChild[idx].ga.maxIndex &&
              entireData[left + entireChild[idx].ga.maxIndex].first <
                  data.first) {
            entireChild[idx].ga.maxIndex++;
            entireData[entireChild[idx].ga.maxIndex + left] = data;
            entireChild[idx].ga.flagNumber++;
            return true;
          }
          if (preIdx > left && entireData[preIdx - 1].first == DBL_MIN) {
            entireData[preIdx - 1] = data;
            entireChild[idx].ga.flagNumber++;
            return true;
          }

          // If the Insertion position is not a gap, we make
          // a gap at the Insertion position by shifting the elements
          // by one position in the direction of the closest gap
          int i = preIdx + 1;
          while (entireData[i].first != DBL_MIN) i++;
          if (i >= left + entireChild[idx].ga.capacity) {
            i = preIdx - 1;
            while (i >= left && entireData[i].first != DBL_MIN) i--;
            for (int j = i; j < preIdx - 1; j++)
              entireData[j] = entireData[j + 1];
            preIdx--;
          } else {
            if (i > entireChild[idx].ga.maxIndex + left)
              entireChild[idx].ga.maxIndex++;
            for (; i > preIdx; i--) {
              entireData[i] = entireData[i - 1];
            }
          }
          entireData[preIdx] = data;
          entireChild[idx].ga.flagNumber++;
          entireChild[idx].ga.maxIndex =
              std::max(entireChild[idx].ga.maxIndex, preIdx - left);
          return true;
        }
        return false;
      }
      case EXTERNAL_ARRAY_LEAF_NODE: {
        // TODO(Jiaoyi): change the right bound.
        int left = entireChild[idx].externalArray.m_left;
        int size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;

        // split
        if (size >= carmi_params::kLeafMaxCapacity) {
          // int previousIdx = entireChild[idx].ga.previousLeaf;
          Split<ExternalArray>(true, left, size, 0, idx);
          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
          left = entireChild[idx].externalArray.m_left;
          size = entireChild[idx].externalArray.flagNumber & 0x00FFFFFF;
        }

        externalData[curr] = data;
        if (size > 0) {
          entireChild[idx].externalArray.flagNumber++;
        } else if (size == 0) {
          entireChild[idx].externalArray.m_left = curr;
          entireChild[idx].externalArray.flagNumber++;
          Train(entireChild[idx].externalArray.m_left, 1, externalData,
                &entireChild[idx].externalArray);
        }
        curr++;
        return true;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_INCLUDE_FUNC_INSERT_FUNCTION_H_
