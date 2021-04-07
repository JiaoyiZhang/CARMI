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

/**
 * @brief allocate empty blocks into emptyBlocks[i]
 *
 * @param left the beginning idx of empty blocks
 * @param len the length of the blocks
 * @return true allocation is successful
 * @return false fails to allocate all blocks
 */
template <typename KeyType, typename ValueType>
inline bool CARMI<KeyType, ValueType>::AllocateEmptyBlock(int left, int len) {
  if (len == 0) return true;
  int res = 0;
  for (int i = emptyBlocks.size() - 1; i >= 0; i--) {
    res = emptyBlocks[i].addBlock(left, len);
    while (res > 0) {
      len = res;
      res = emptyBlocks[i].addBlock(left, len);
    }
    if (res == 0) {
      return true;
    }
  }
  return false;
}

/**
 * @brief find the actual size in emptyBlocks
 *
 * @param size the size of the data points
 * @return int the index in emptyBlocks
 */
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

/**
 * @brief find the corresponding index in emptyBlocks
 *
 * @param size the size of the data points
 * @return int the index in emptyBlocks
 */
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

/**
 * @brief initialize entireData
 *
 * @param left the left index of data points needed to be allocated
 * @param size the size of data points
 * @param reinit whether the entireData has been initialized
 */
template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::InitEntireData(int left, int size,
                                               bool reinit) {
  unsigned int len = 4096;
  while (len < size) len *= 2;
  len *= 2;
  entireDataSize = len;

#ifdef DEBUG
  std::cout << "dataset size:" << size << std::endl;
  std::cout << "the size of entireData is:" << len << std::endl;
#endif  // DEBUG

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
  if (reinit) len = size;
  auto res = AllocateEmptyBlock(left, len);
}

/**
 * @brief allocate a block to this leaf node
 *
 * @param size the size of the leaf node needs to be allocated
 * @param idx the idx in emptyBlocks
 * @return int return idx (if it fails, return -1)
 */
template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateSingleMemory(int size, int *idx) {
  auto newLeft = -1;
  for (int i = *idx; i < emptyBlocks.size(); i++) {
    newLeft = emptyBlocks[i].allocate(size);
    if (newLeft != -1) {
      // split the block to smaller blocks
      if (i > *idx) {
        AllocateEmptyBlock(newLeft, size);
        newLeft = emptyBlocks[*idx].allocate(size);
      }

      *idx = i;
      break;
    }
  }
  return newLeft;
}

/**
 * @brief allocate a block to the current leaf node
 *
 * @param size the size of the leaf node needs to be allocated
 * @return int return idx (if it fails, return -1)
 */
template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateMemory(int size) {
  int idx = GetIndex(size);  // idx in emptyBlocks[]
  size = emptyBlocks[idx].m_width;

  auto newLeft = AllocateSingleMemory(size, &idx);
  // allocation fails
  // need to expand the reorganize entireData
  if (newLeft == -1) {
#ifdef DEBUG
    std::cout << "need expand the entire!" << std::endl;
#endif  // DEBUG

    unsigned int tmpSize = entireDataSize;
    DataVectorType tmpData = entireData;
    std::vector<EmptyBlock> tmpBlocks = emptyBlocks;

    InitEntireData(tmpSize, tmpSize, true);
    for (int i = 0; i < tmpSize; i++) {
      entireData[i] = tmpData[i];
    }
    for (int i = 0; i < emptyBlocks.size(); i++)
      emptyBlocks[i].m_block.insert(tmpBlocks[i].m_block.begin(),
                                    tmpBlocks[i].m_block.end());
    newLeft = AllocateSingleMemory(size, &idx);
  }

  // add the blocks into the corresponding blocks
  auto res =
      AllocateEmptyBlock(newLeft + size, emptyBlocks[idx].m_width - size);
  if (newLeft + size > nowDataSize) {
    nowDataSize = newLeft + size;
  }
  return newLeft;
}

/**
 * @brief release the specified space
 *
 * @param left the left index
 * @param size the size
 */
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
