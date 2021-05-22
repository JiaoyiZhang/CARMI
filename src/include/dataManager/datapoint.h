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

#include <iostream>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./empty_block.h"

template <typename KeyType, typename ValueType>
inline bool CARMI<KeyType, ValueType>::AllocateEmptyBlock(int left, int len) {
  if (len == 0) return true;
  int res = 0;
  for (int i = emptyBlocks.size() - 1; i >= 0; i--) {
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
  if (size <= 1) return 1;

  for (int j = emptyBlocks.size() - 1; j > 0; j--) {
    if (size <= emptyBlocks[j].m_width && size > emptyBlocks[j - 1].m_width) {
      return emptyBlocks[j].m_width;
    }
  }
  return 0;
}

template <typename KeyType, typename ValueType>
inline int CARMI<KeyType, ValueType>::GetIndex(int size) {
  if (size <= 1) {
    return 0;
  }

  for (int j = emptyBlocks.size() - 1; j > 0; j--) {
    if (size <= emptyBlocks[j].m_width && size > emptyBlocks[j - 1].m_width) {
      return j;
    }
  }
  return -1;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::InitEntireData(int size) {
  unsigned int len = 4096;
  while (len < size) len *= 2;
  len *= 2;
  entireDataSize = len;

  std::vector<EmptyBlock>().swap(emptyBlocks);
  DataVectorType().swap(entireData);
  entireData = DataVectorType(len, {DBL_MIN, DBL_MIN});
  emptyBlocks.push_back(EmptyBlock(1));
  emptyBlocks.push_back(EmptyBlock(2));
  emptyBlocks.push_back(EmptyBlock(3));
  for (int i = 1; i < 11; i++) {
    emptyBlocks.push_back(EmptyBlock(2 * pow(2, i)));
    emptyBlocks.push_back(EmptyBlock(3 * pow(2, i)));
  }
  emptyBlocks.push_back(EmptyBlock(4096));
  auto res = AllocateEmptyBlock(0, len);
}

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateSingleMemory(int size, int *idx) {
  auto newLeft = -1;
  for (int i = *idx; i < static_cast<int>(emptyBlocks.size()); i++) {
    newLeft = emptyBlocks[i].allocate(size);
    if (newLeft != -1) {
      *idx = i;
      break;
    }
  }
  return newLeft;
}

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateMemory(int size) {
  int idx = GetIndex(size);  // idx in emptyBlocks[]
  size = emptyBlocks[idx].m_width;

  auto newLeft = AllocateSingleMemory(size, &idx);

  // allocation fails
  // need to expand the entireData
  if (newLeft == -1) {
    entireData.resize(entireDataSize * 2, {DBL_MIN, DBL_MIN});
    AllocateEmptyBlock(entireDataSize, entireDataSize);
    entireDataSize *= 2;
    newLeft = AllocateSingleMemory(size, &idx);
  }

  // if the allocated size is less than block size, add the rest empty blocks
  // into the corresponding blocks
  if (size < emptyBlocks[idx].m_width) {
    AllocateEmptyBlock(newLeft + size, emptyBlocks[idx].m_width - size);
  }

  // update the right bound of data points in the entireData
  if (newLeft + size > static_cast<int>(nowDataSize)) {
    nowDataSize = newLeft + size;
  }
  return newLeft;
}

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::ReleaseMemory(int left, int size) {
  int idx = GetIndex(size);
  for (int i = idx; i < emptyBlocks.size(); i++) {
    if (!emptyBlocks[i].find(left + emptyBlocks[i].m_width)) {
      emptyBlocks[i].m_block.insert(left);
      break;
    }
  }
}

#endif  // SRC_INCLUDE_DATAMANAGER_DATAPOINT_H_
