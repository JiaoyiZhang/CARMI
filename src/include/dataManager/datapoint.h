/**
 * @file datapoint.h
 * @author Jiaoyi
 * @brief manage the entireData array
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_DATAMANAGER_DATAPOINT_H_
#define SRC_INCLUDE_DATAMANAGER_DATAPOINT_H_
#include <float.h>
#include <math.h>

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./empty_block.h"

template <typename KeyType, typename ValueType>
inline bool CARMI<KeyType, ValueType>::AllocateEmptyBlock(int left, int len) {
  if (len == 0) return true;
  int res = 0;
  for (int i = emptyBlocks.size() - 1; i > 0; i--) {
    res = emptyBlocks[i].addBlock(left, len);
    while (res > 0) {
      len = res;
      res = emptyBlocks[i].addBlock(left, len);
      if (res == -1) {
        break;
      }
    }
    if (res == 0) {
      return true;
    }
  }
  return false;
}

template <typename KeyType, typename ValueType>
inline int CARMI<KeyType, ValueType>::GetActualSize(int size) {
  if (size <= 0) return 0;
  if (size > kMaxLeafNum * kMaxSlotNum) {
#ifdef DEBUG
    std::cout << "the size is " << size
              << ",\tthe maximum size in a leaf node is "
              << kMaxLeafNum * kMaxSlotNum << std::endl;
#endif  // DEBUG
    return -1;
  }
  int leafNumber = std::min(
      static_cast<int>(ceil(size / readRate / kMaxSlotNum)), kMaxLeafNum);

  return leafNumber;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::InitEntireData(int size) {
  entireDataSize = std::max(64.0, size / carmi_params::kMaxLeafNodeSize * 1.5);
  std::vector<LeafSlots<KeyType, ValueType>>().swap(entireData);
  entireData = std::vector<LeafSlots<KeyType, ValueType>>(
      entireDataSize, LeafSlots<KeyType, ValueType>());
  unsigned int len = 4096;
  while (len < static_cast<unsigned int>(size)) len *= 2;
  len = len * 2.5 / kMaxSlotNum;
  entireDataSize = len;

  std::vector<EmptyBlock>().swap(emptyBlocks);
  std::vector<LeafSlots<KeyType, ValueType>>().swap(entireData);
  entireData = std::vector<LeafSlots<KeyType, ValueType>>(
      len, LeafSlots<KeyType, ValueType>());
  for (int i = 0; i <= kMaxLeafNum; i++) {
    emptyBlocks.push_back(EmptyBlock(i));
  }
  AllocateEmptyBlock(0, len);
}

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateSingleMemory(int *idx) {
  int newLeft = -1;
  for (int i = *idx; i < static_cast<int>(emptyBlocks.size()); i++) {
    newLeft = emptyBlocks[i].allocate();
    if (newLeft != -1) {
      *idx = i;
      break;
    }
  }
  return newLeft;
}

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateMemory(int neededLeafNumber) {
  int idx = neededLeafNumber;
  int newLeft = AllocateSingleMemory(&idx);

  // allocation fails
  // need to expand the entireData
  if (newLeft == -1) {
    entireData.resize(entireDataSize * 2, LeafSlots<KeyType, ValueType>());
    AllocateEmptyBlock(entireDataSize, entireDataSize);
    entireDataSize *= 2;
    newLeft = AllocateSingleMemory(&idx);
  }

  // if the allocated size is less than block size, add the rest empty blocks
  // into the corresponding blocks
  if (neededLeafNumber < emptyBlocks[idx].m_width) {
    AllocateEmptyBlock(newLeft + neededLeafNumber,
                       emptyBlocks[idx].m_width - neededLeafNumber);
  }

  // update the right bound of data points in the entireData
  if (newLeft + neededLeafNumber > static_cast<int>(nowDataSize)) {
    nowDataSize = newLeft + neededLeafNumber;
  }
  return newLeft;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::ReleaseMemory(int left, int size) {
  // emptyBlocks[size].m_block.insert(left);

  int len = size;
  int idx = 1;
  while (idx < 7) {
    if (emptyBlocks[idx].find(left + len)) {
      emptyBlocks[idx].m_block.erase(left + len);
      len += emptyBlocks[idx].m_width;
      idx = 1;
    } else {
      idx++;
    }
  }
  AllocateEmptyBlock(left, len);
}

#endif  // SRC_INCLUDE_DATAMANAGER_DATAPOINT_H_
