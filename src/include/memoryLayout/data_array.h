/**
 * @file data_array.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-10-11
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef MEMORYLAYOUT_DATA_ARRAY_H_
#define MEMORYLAYOUT_DATA_ARRAY_H_
#include <algorithm>
#include <vector>

#include "../construct/structures.h"
#include "../params.h"
#include "./empty_block.h"

/**
 * @brief the structure of data array
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class DataArrayStructure {
 public:
  DataArrayStructure() {
    nowDataSize = 0;
    totalDataSize = 0;
  }

  void InitDataArray(int size);

  void Resize(int num) {
    dataArray.resize(num, LeafSlots<KeyType, ValueType>());
    totalDataSize = num;
    nowDataSize = 0;
  }

  /**
   * @brief Set the MaxBlockNum object
   *
   * @param n the maximum block number
   */
  void SetMaxBlockNum(int n) { maxBlockNum = n; }

  /**
   * @brief add the index of empty memory blocks into emptyBlocks[i]
   *
   * @param left the beginning idx of empty blocks
   * @param len the length of the blocks
   * @retval true allocation is successful
   * @retval false fails to allocate all blocks
   */
  bool AddEmptyBlock(int left, int len);

  /**
   * @brief allocate a block of memory to this leaf node
   *
   * @param idx the idx in emptyBlocks
   * @return int: the start index of the allocation
   * @retval -1 allocation fails
   */
  int AllocateSingleMemory(int *idx);

  /**
   * @brief allocate a block of memory to the leaf node
   *
   * @param neededBlockNumber the number of data blocks needed to be allocated
   * @return int: return idx
   * @retval -1 allocation fails
   */
  int AllocateMemory(int neededBlockNumber);

  /**
   * @brief release the specified spacse
   *
   * @param left the left index
   * @param size the size
   */
  void ReleaseMemory(int left, int size);

  /**
   * @brief release the useless memory
   *
   * @param neededSize the size of needed data blocks
   */
  void ReleaseUselessMemory(int neededSize);

  std::vector<LeafSlots<KeyType, ValueType>> dataArray;  ///< the data array
  int totalDataSize;  ///< the total size of the data array
  int nowDataSize;    ///< the used size of data to store data points

 private:
  /// store the index of all empty,blocks(size: 1,2^i, 3*2^i, 4096)
  std::vector<EmptyBlock> emptyBlocks;
  int maxBlockNum;  ///< the maximum number of data blocks in a leaf node
};

template <typename KeyType, typename ValueType>
void DataArrayStructure<KeyType, ValueType>::InitDataArray(int size) {
  nowDataSize = 0;
  totalDataSize = std::max(64.0, size / carmi_params::kMaxLeafNodeSize * 1.5);
  std::vector<LeafSlots<KeyType, ValueType>>().swap(dataArray);
  dataArray = std::vector<LeafSlots<KeyType, ValueType>>(
      totalDataSize, LeafSlots<KeyType, ValueType>());

  std::vector<EmptyBlock>().swap(emptyBlocks);
  for (int i = 0; i <= maxBlockNum; i++) {
    emptyBlocks.push_back(EmptyBlock(i));
  }
}

template <typename KeyType, typename ValueType>
bool DataArrayStructure<KeyType, ValueType>::AddEmptyBlock(int left, int len) {
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
int DataArrayStructure<KeyType, ValueType>::AllocateSingleMemory(int *idx) {
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
int DataArrayStructure<KeyType, ValueType>::AllocateMemory(
    int neededBlockNumber) {
  int newLeft = -1;
  int idx = neededBlockNumber;
  newLeft = AllocateSingleMemory(&idx);
  // allocation fails, need to expand the data
  if (newLeft == -1) {
    dataArray.resize(totalDataSize * 1.5, LeafSlots<KeyType, ValueType>());
    AddEmptyBlock(totalDataSize, totalDataSize * 0.5);
    totalDataSize *= 1.5;
    newLeft = AllocateSingleMemory(&idx);
  }

  // if the allocated size is less than block size, add the rest empty blocks
  // into the corresponding blocks
  if (neededBlockNumber < emptyBlocks[idx].m_width) {
    AddEmptyBlock(newLeft + neededBlockNumber,
                  emptyBlocks[idx].m_width - neededBlockNumber);
  }

  // update the right bound of data points in the data
  if (newLeft + neededBlockNumber > static_cast<int>(nowDataSize)) {
    nowDataSize = newLeft + neededBlockNumber;
  }
  return newLeft;
}

template <typename KeyType, typename ValueType>
void DataArrayStructure<KeyType, ValueType>::ReleaseMemory(int left, int size) {
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
  AddEmptyBlock(left, len);
}

template <typename KeyType, typename ValueType>
void DataArrayStructure<KeyType, ValueType>::ReleaseUselessMemory(
    int neededSize) {
  if (neededSize < static_cast<int>(dataArray.size())) {
    std::vector<LeafSlots<KeyType, ValueType>> tmpEntireData(
        dataArray.begin(), dataArray.begin() + neededSize);
    std::vector<LeafSlots<KeyType, ValueType>>().swap(dataArray);
    dataArray = tmpEntireData;
  }

  for (int i = 0; i < static_cast<int>(emptyBlocks.size()); i++) {
    auto it = emptyBlocks[i].m_block.lower_bound(neededSize);
    emptyBlocks[i].m_block.erase(it, emptyBlocks[i].m_block.end());
    auto tmp = emptyBlocks[i];
    for (auto j = tmp.m_block.begin(); j != tmp.m_block.end(); j++) {
      if (tmp.m_width + *j > static_cast<int>(dataArray.size())) {
        AddEmptyBlock(*j, dataArray.size() - *j);
        emptyBlocks[i].m_block.erase(*j);
        break;
      }
    }
  }
}

#endif  // MEMORYLAYOUT_DATA_ARRAY_H_
