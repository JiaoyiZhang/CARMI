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
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../construct/structures.h"
#include "../params.h"
#include "./empty_block.h"

/**
 * @brief the structure of data array, used to store data points in CARMI.
 *
 * This class is used to store data points in CARMI. This class includes the
 * management of the data array mentioned in the paper, including modifying the
 * size of the data array, allocating or releasing a contiguous memory space for
 * the leaf node, and so on.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 * @tparam Alloc Type of the allocator object used to define the storage
 * allocation model.
 */
template <typename KeyType, typename ValueType,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
class DataArrayStructure {
 private:
  /**
   * @brief Define an related allocator for the leaf_node structs.
   */
  typedef typename Alloc::template rebind<DataArrayStructure>::other alloc_type;

  /**
   * @brief The allocator for data vector.
   */
  Alloc m_allocator;

 public:
  //*** Constructor

  /**
   * @brief Construct a new Data Array Structure object, set the usedDatasize
   * member as 0
   *
   * The externalArray leaf nodes don't need this class.
   */
  DataArrayStructure() { usedDatasize = 0; }

  /**
   * @brief Construct a new Data Array Structure object
   *
   * Initialize dataArray and update its related member variables according to
   * the size of the initial dataset. In addition, initialize the various size
   * of empty memory blocks according to the maxBlockNum parameter.
   *
   * @param[in] maxBlockNum the maximum width of the empty memory block, which
   * is equal to the kMaxBlockNum in CF array leaf node.
   * @param[in] size the size of the init dataset
   */
  DataArrayStructure(int maxBlockNum, int size) {
    if (maxBlockNum <= 0) {
      throw std::logic_error(
          "DataArrayStructure: the maximum block number is invalid.");
    }
    usedDatasize = 0;
    int totalDataSize = std::max(
        64.0, static_cast<double>(size) / carmi_params::kMaxLeafNodeSize * 1.5);
    std::vector<LeafSlots<KeyType, ValueType>, Alloc>().swap(dataArray);
    dataArray = std::vector<LeafSlots<KeyType, ValueType>, Alloc>(
        totalDataSize, LeafSlots<KeyType, ValueType>());

    std::vector<EmptyMemoryBlock>().swap(emptyBlocks);
    for (int i = 0; i <= maxBlockNum; i++) {
      emptyBlocks.push_back(EmptyMemoryBlock(i));
    }
  }

 public:
  //*** Public Functions of DataArrayStructure

  /**
   * @brief add empty memory blocks ([left, left + len]) with the length len
   * into emptyBlocks
   *
   * This function will split the empty memory block with the given length into
   * different small blocks and store them in the corresponding members of
   * emptyBlocks, thus realizing classification management.
   *
   * @param[in] left the beginning idx of this block of empty memory
   * @param[in] len the length of the empty memory block
   */
  void AddEmptyMemoryBlock(int left, int len);

  /**
   * @brief allocate a block of empty memory to the leaf node
   *
   * @param[in] neededBlockNumber the number of data blocks needed to be
   * allocated
   * @return int: return the beginning index of the allocated memory block
   * @retval -1 allocation fails, there are no more empty memory blocks.
   */
  int AllocateMemory(int neededBlockNumber);

  /**
   * @brief release the specified space [left, left + size], and insert the left
   * index into emptyBlocks for future reuse.
   *
   * @param[in] left the left index of the useless memory block
   * @param[in] size the size of the useless memory block
   */
  void ReleaseMemory(int left, int size);

  /**
   * @brief After the construction of CARMI is completed, the useless memory
   * exceeding the needed size will be released.
   *
   * @param[in] neededSize the size of needed data blocks
   */
  void ReleaseUselessMemory(int neededSize);

 private:
  //*** Private Functions of Data Array Structure Objects

  /**
   * @brief allocate a block of empty memory to this leaf node
   *
   * @param[in] idx the width of allocated memory block. Based on our design,
   * the width of the empty memory block is equal to its index.
   * @return int: the beginning index of the allocated memory block
   * @retval -1 allocation fails
   */
  int AllocateSingleMemory(int idx);

  /**
   * @brief Return the data allocator
   *
   * @return alloc_type
   */
  alloc_type data_allocator() { return alloc_type(m_allocator); }

 public:
  //*** Public Data Member of Data Array Structure Objects

  /**
   * @brief used to store data points in CARMI.
   *
   * dataArray is a large array containing many small data blocks, each of which
   * has a fixed size. The fixed structure is determined by the design of
   * LeafSlots class. Data blocks managed by the same leaf node are stored in
   * adjacent locations.
   */
  std::vector<LeafSlots<KeyType, ValueType>, Alloc> dataArray;

  /**
   * @brief the used size of dataArray.
   *
   * The used size of dataArray, which is represented by the index of the last
   * used element in the dataArray plus one. It is worth noting that even if
   * some gaps in the middle have not been used, because there are some used
   * parts behind them, we also include their size in this parameter.
   */
  int usedDatasize;

 private:
  //*** Private Data Member of Data Array Structure Objects

  /**
   * @brief store the starting index of all empty memory blocks according to
   * different widths
   */
  std::vector<EmptyMemoryBlock> emptyBlocks;
};

template <typename KeyType, typename ValueType, typename Alloc>
void DataArrayStructure<KeyType, ValueType, Alloc>::AddEmptyMemoryBlock(
    int left, int len) {
  // Case 1: len is equal to 0, return directly
  if (len <= 0) return;
  if (left < 0 || left + len > dataArray.size()) {
    throw std::out_of_range(
        "DataArrayStructure::AddEmptyMemoryBlock: the range is invalid.");
  }
  int res = 0;
  // Case 2: split the block into several small blocks and store them in the
  // corresponding members of emptyBlocks
  for (int i = emptyBlocks.size() - 1; i > 0; i--) {
    res = emptyBlocks[i].AddBlock(left, len);
    while (res > 0) {
      len = res;
      res = emptyBlocks[i].AddBlock(left, len);
      if (res == -1) {
        break;
      }
    }
    // Case 2.1: there are no more blocks, return
    if (res == 0) {
      return;
    }
  }
}

template <typename KeyType, typename ValueType, typename Alloc>
int DataArrayStructure<KeyType, ValueType, Alloc>::AllocateSingleMemory(
    int idx) {
  if (idx < 0 || idx > emptyBlocks[emptyBlocks.size() - 1].m_width) {
    throw std::invalid_argument(
        "DataArrayStructure::AllocateSingleMemory: the idx is invalid.");
  }
  int newLeft = -1;
  for (int i = idx; i < static_cast<int>(emptyBlocks.size()); i++) {
    newLeft = emptyBlocks[i].Allocate();
    if (newLeft != -1) {
      if (i != idx) {
        // if the needed width is less than block width, insert the rest
        // empty blocks into the corresponding emptyBlocks
        AddEmptyMemoryBlock(newLeft + idx, emptyBlocks[i].m_width - idx);
      }
      break;
    }
  }
  return newLeft;
}

template <typename KeyType, typename ValueType, typename Alloc>
int DataArrayStructure<KeyType, ValueType, Alloc>::AllocateMemory(
    int neededBlockNumber) {
  int newLeft = -1;
  int idx = neededBlockNumber;
  newLeft = AllocateSingleMemory(idx);

  // allocation fails, need to expand the dataArray
  if (newLeft == -1) {
    int oldDatasize = dataArray.size();
    dataArray.resize(oldDatasize * 1.25, LeafSlots<KeyType, ValueType>());
    AddEmptyMemoryBlock(oldDatasize, oldDatasize * 0.25);
    newLeft = AllocateSingleMemory(idx);
  }

  // update the right bound (usedDatasize) of data points in the data
  if (newLeft + neededBlockNumber > static_cast<int>(usedDatasize)) {
    usedDatasize = newLeft + neededBlockNumber;
  }
  return newLeft;
}

template <typename KeyType, typename ValueType, typename Alloc>
void DataArrayStructure<KeyType, ValueType, Alloc>::ReleaseMemory(int left,
                                                                  int size) {
  if (left < 0 || size < 0 || left + size > dataArray.size()) {
    throw std::out_of_range(
        "DataArrayStructure::ReleaseMemory: the empty memory block is "
        "invalid.");
  }
  int len = size;
  int idx = 1;
  // traverse all members and delete all indexes within the range
  while (idx < emptyBlocks.size()) {
    if (emptyBlocks[idx].IsEmpty(left + len)) {
      emptyBlocks[idx].m_block.erase(left + len);
      len += emptyBlocks[idx].m_width;
      idx = 1;
    } else {
      idx++;
    }
  }
  AddEmptyMemoryBlock(left, len);
}

template <typename KeyType, typename ValueType, typename Alloc>
void DataArrayStructure<KeyType, ValueType, Alloc>::ReleaseUselessMemory(
    int neededSize) {
  if (neededSize < 0) {
    throw std::invalid_argument(
        "DataArrayStructure::ReleaseUselessMemory: the useless memory block is "
        "invalid.");
  }
  // release the useless memory of dataArray
  if (neededSize < static_cast<int>(dataArray.size())) {
    std::vector<LeafSlots<KeyType, ValueType>, Alloc> tmpEntireData(
        dataArray.begin(), dataArray.begin() + neededSize);
    std::vector<LeafSlots<KeyType, ValueType>, Alloc>().swap(dataArray);
    dataArray = tmpEntireData;
  }

  // release the corresponding beginning index of the useless memory blocks in
  // emptyBlocks
  for (int i = 0; i < static_cast<int>(emptyBlocks.size()); i++) {
    auto it = emptyBlocks[i].m_block.lower_bound(neededSize);
    emptyBlocks[i].m_block.erase(it, emptyBlocks[i].m_block.end());
    auto tmp = emptyBlocks[i];
    for (auto j = tmp.m_block.begin(); j != tmp.m_block.end(); j++) {
      if (tmp.m_width + *j > static_cast<int>(dataArray.size())) {
        AddEmptyMemoryBlock(*j, dataArray.size() - *j);
        emptyBlocks[i].m_block.erase(*j);
        break;
      }
    }
  }
}

#endif  // MEMORYLAYOUT_DATA_ARRAY_H_
