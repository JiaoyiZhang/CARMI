/**
 * @file datapoint.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_DATAMANAGER_DATAPOINT_H_
#define SRC_CARMI_DATAMANAGER_DATAPOINT_H_
#include <float.h>
#include <math.h>

#include <iostream>
#include <utility>
#include <vector>

#include "../carmi.h"
#include "./empty_block.h"

// allocate empty blocks into emptyBlocks[i]
// left: the beginning idx of empty blocks
// len: the length of the blocks
inline bool CARMI::allocateEmptyBlock(int left, int len) {
  if (len == 0) return true;
  int res = 0;
  for (int i = 12; i >= 0; i--) {
    res = emptyBlocks[i].addBlock(left, len);
    while (res > 0) {
      len = res;
      res = emptyBlocks[i].addBlock(left, len);
    }
    if (res == 0) return true;
  }
  return false;
}

// find the corresponding index in emptyBlocks
// return idx
inline int CARMI::getIndex(int size) {
#ifdef DEBUG
  if (size > 4096 || size < 1)
    std::cout << "size: " << size
              << ",\tsize > 4096 || size < 1, getIndex WRONG!" << std::endl;
#endif  // DEBUG
  int j = 4096;
  for (int i = 12; i >= 0; i--, j /= 2) {
    if (size <= j && size > j / 2) return i;
  }
  return -1;
}

// initialize entireData
void CARMI::initEntireData(int left, int size, bool reinit) {
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
  for (int i = 0, j = 1; i < 13; i++, j *= 2)
    emptyBlocks.push_back(EmptyBlock(j));
  if (reinit) len = size;
  auto res = allocateEmptyBlock(left, len);
#ifdef DEBUG
  if (!res) std::cout << "init allocateEmptyBlock WRONG!" << std::endl;
#endif  // DEBUG
}

// allocate a block to the current leaf node
// size: the size of the leaf node needs to be allocated
// return the starting position of the allocation
// return -1, if it fails
int CARMI::allocateMemory(int size) {
  int idx = getIndex(size);  // idx in emptyBlocks[]
  size = emptyBlocks[idx].m_width;
#ifdef DEBUG
  if (idx == -1)
    std::cout << "getIndex in emptyBlocks WRONG!\tsize:" << size << std::endl;
#endif  // DEBUG
  auto newLeft = -1;
  for (int i = idx; i < 13; i++) {
    newLeft = emptyBlocks[i].allocate(size);
    if (newLeft != -1) {
      idx = i;
      break;
    }
  }
  // allocation fails
  // need to expand the reorganize entireData
  if (newLeft == -1) {
#ifdef DEBUG
    std::cout << "need expand the entire!" << std::endl;
#endif  // DEBUG
    unsigned int tmpSize = entireDataSize;
    DataVectorType tmpData = entireData;
    std::vector<EmptyBlock> tmpBlocks = emptyBlocks;

    initEntireData(tmpSize, tmpSize, true);
    for (int i = 0; i < tmpSize; i++) entireData[i] = tmpData[i];
    for (int i = 0; i < 13; i++)
      emptyBlocks[i].m_block.insert(tmpBlocks[i].m_block.begin(),
                                    tmpBlocks[i].m_block.end());
    for (int i = idx; i < 13; i++) {
      newLeft = emptyBlocks[i].allocate(size);
      if (newLeft != -1) {
        idx = i;
        break;
      }
    }
  }

  // add the left blocks into the corresponding blocks
  auto res =
      allocateEmptyBlock(newLeft + size, emptyBlocks[idx].m_width - size);
  if (newLeft + size > nowDataSize) nowDataSize = newLeft + size;
  return newLeft;
}

// release the specified space
void CARMI::releaseMemory(int left, int size) {
  int idx = getIndex(size);
  for (int i = idx; i < 13; i++) {
    if (!emptyBlocks[i].find(left + emptyBlocks[i].m_width)) {
      emptyBlocks[i].m_block.insert(left);
      break;
    }
  }
}

#endif  // SRC_CARMI_DATAMANAGER_DATAPOINT_H_
