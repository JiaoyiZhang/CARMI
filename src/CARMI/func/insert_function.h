/**
 * @file insert_function.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_FUNC_INSERT_FUNCTION_H_
#define SRC_CARMI_FUNC_INSERT_FUNCTION_H_

#include <float.h>

#include <algorithm>
#include <map>
#include <utility>
#include <vector>

#include "../carmi.h"

bool CARMI::Insert(DataType data) {
  int idx = 0;  // idx in the INDEX
  int content;
  int type = rootType;
  int childIdx = 0;
  while (1) {
    switch (type) {
      case LR_ROOT_NODE:
        idx = root.lrRoot.childLeft + root.lrRoot.model->Predict(data.first);
        break;
      case PLR_ROOT_NODE:
        idx = root.plrRoot.childLeft + root.plrRoot.model->Predict(data.first);
        break;
      case HIS_ROOT_NODE:
        idx = root.hisRoot.childLeft + root.hisRoot.model->Predict(data.first);
        break;
      case BS_ROOT_NODE:
        idx = root.bsRoot.childLeft + root.bsRoot.model->Predict(data.first);
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
        auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
        // split
        if (size >= kLeafMaxCapacity) {
          DataVectorType tmpDataset;
          auto left = entireChild[idx].array.m_left;
          auto size = entireChild[idx].array.flagNumber & 0x00FFFFFF;
          for (int i = left; i < left + size; i++)
            tmpDataset.push_back(entireData[i]);

          auto node = LRModel();  // create a new inner node
          int childNum = kInsertNewChildNumber;
          node.SetChildNumber(kInsertNewChildNumber);
          node.childLeft = AllocateChildMemory(childNum);
          Train(0, tmpDataset.size(), tmpDataset, &node);
          entireChild[idx].lr = node;

          std::vector<DataVectorType> subFindData;
          DataVectorType tmp;
          for (int i = 0; i < childNum; i++) subFindData.push_back(tmp);

          for (int i = 0; i < size; i++) {
            int p = node.Predict(tmpDataset[i].first);
            subFindData[p].push_back(tmpDataset[i]);
          }

          for (int i = 0; i < childNum; i++) {
            ArrayType tmpLeaf(kThreshold);
            InitArray(kMaxKeyNum, left, 1, subFindData[i], &tmpLeaf);
            entireChild[node.childLeft + i].array = tmpLeaf;
          }
          auto previousIdx = entireChild[idx].array.previousLeaf;
          entireChild[previousIdx].array.nextLeaf = node.childLeft;
          entireChild[node.childLeft].array.previousLeaf = previousIdx;
          for (int i = node.childLeft + 1; i < node.childLeft + childNum - 1;
               i++) {
            entireChild[i].array.previousLeaf = i - 1;
            entireChild[i].array.nextLeaf = i + 1;
          }
          entireChild[node.childLeft + childNum - 1].array.previousLeaf =
              node.childLeft + childNum - 2;

          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
        }
        auto left = entireChild[idx].array.m_left;
        // size == 0
        if (size == 0) {
          entireData[left] = data;
          entireChild[idx].array.flagNumber++;
          InitArray(entireChild[idx].array.m_capacity, left, 1, entireData,
                    &entireChild[idx].array);
          return true;
        }
        int preIdx = entireChild[idx].array.Predict(data.first);
        preIdx = ArraySearch(data.first, preIdx, entireChild[idx].array.error,
                             left, size);

        // expand
        if ((size >= entireChild[idx].array.m_capacity) &&
            entireChild[idx].array.m_capacity < kLeafMaxCapacity) {
          auto diff = preIdx - left;
          InitArray(entireChild[idx].array.m_capacity, left, 1, entireData,
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
        auto left = entireChild[idx].ga.m_left;
        int size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
        // split
        if (size >= kLeafMaxCapacity) {
          DataVectorType tmpDataset;
          auto left = entireChild[idx].ga.m_left;
          auto size = entireChild[idx].ga.flagNumber & 0x00FFFFFF;
          for (int i = left; i < left + size; i++)
            tmpDataset.push_back(entireData[i]);

          auto node = LRModel();  // create a new inner node
          node.SetChildNumber(kInsertNewChildNumber);
          int childNum = kInsertNewChildNumber;
          node.childLeft = AllocateChildMemory(childNum);
          Train(0, tmpDataset.size(), tmpDataset, &node);
          entireChild[idx].lr = node;

          std::vector<DataVectorType> subFindData;
          DataVectorType tmp;
          for (int i = 0; i < childNum; i++) subFindData.push_back(tmp);

          for (int i = 0; i < size; i++) {
            int p = node.Predict(tmpDataset[i].first);
            subFindData[p].push_back(tmpDataset[i]);
          }

          for (int i = 0; i < childNum; i++) {
            GappedArrayType tmpLeaf(kThreshold);
            InitGA(kMaxKeyNum, 0, subFindData[i].size(), subFindData[i],
                   &tmpLeaf);
            entireChild[node.childLeft + i].ga = tmpLeaf;
          }

          auto previousIdx = entireChild[idx].ga.previousLeaf;
          entireChild[previousIdx].ga.nextLeaf = node.childLeft;
          entireChild[node.childLeft].ga.previousLeaf = previousIdx;
          for (int i = node.childLeft + 1; i < node.childLeft + childNum - 1;
               i++) {
            entireChild[i].ga.previousLeaf = i - 1;
            entireChild[i].ga.nextLeaf = i + 1;
          }
          entireChild[node.childLeft + childNum - 1].ga.previousLeaf =
              node.childLeft + childNum - 2;

          idx = entireChild[idx].lr.childLeft +
                entireChild[idx].lr.Predict(data.first);
        }
        // expand
        if (entireChild[idx].ga.capacity < kLeafMaxCapacity &&
            (static_cast<float>(size) / entireChild[idx].ga.capacity >
             entireChild[idx].ga.density)) {
          // If an additional Insertion results in crossing the density
          // then we expand the gapped array
          DataVectorType newDataset;
          int right = left + (&entireChild[idx].ga)->maxIndex + 1;
          for (int i = left; i < right; i++) {
            newDataset.push_back(entireData[i]);
          }
          InitGA(entireChild[idx].ga.capacity, left, size, newDataset,
                 &entireChild[idx].ga);
        }
        // size == 0
        if (size == 0) {
          entireData[left] = data;
          entireChild[idx].ga.flagNumber++;
          entireChild[idx].ga.maxIndex = 0;
          InitGA(entireChild[idx].ga.capacity, left, 1, entireData,
                 &entireChild[idx].ga);
          return true;
        }
        // find position
        int preIdx = entireChild[idx].ga.Predict(data.first);
        preIdx =
            ExternalSearch(data.first, preIdx,
                           entireChild[idx].externalArray.error, left, size);

        // if the Insertion position is a gap,
        //  then we Insert the element into the gap and are done
        if (entireData[preIdx].first == DBL_MIN) {
          entireData[preIdx] = data;
          entireChild[idx].ga.flagNumber++;
          entireChild[idx].ga.maxIndex =
              std::max(entireChild[idx].ga.maxIndex, preIdx - left);
          return true;
        } else {
          if (entireData[preIdx - 1].first == DBL_MIN) {
            entireData[preIdx - 1] = data;
            entireChild[idx].ga.flagNumber++;
            return true;
          }
          if (preIdx == left + entireChild[idx].ga.maxIndex &&
              entireData[left + entireChild[idx].ga.maxIndex].first <
                  data.first) {
            entireChild[idx].ga.maxIndex = entireChild[idx].ga.maxIndex + 1;
            entireData[entireChild[idx].ga.maxIndex + left] = data;
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
            for (; i > preIdx; i--) entireData[i] = entireData[i - 1];
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
        externalData[curr] = data;
        curr++;
        return true;
      }
    }

    type = entireChild[idx].lr.flagNumber >> 24;
  }
}

#endif  // SRC_CARMI_FUNC_INSERT_FUNCTION_H_
