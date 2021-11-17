/**
 * @file empty_block.h
 * @author Jiaoyi
 * @brief the class of empty memory blocks
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef MEMORYLAYOUT_EMPTY_BLOCK_H_
#define MEMORYLAYOUT_EMPTY_BLOCK_H_

#include <iostream>
#include <set>

/**
 * @brief Basic class used to manage empty memory blocks.
 *
 * This class is used to manage empty memory blocks with the size m_width.
 * In CARMI, this class can speed up the process of memory allocation, which
 * only needs to return the first element in m_block.
 *
 * This class is used as a member type of the vector of the DataArrayStructure
 * in data_array.h. Users can customize the granularity of the width of the
 * empty memory blocks according to the node type they implement. For example,
 * these can be 1~7 for the CF array leaf node. At the same time, it can also
 * have a coarser granularity:2, 4, 8, ..., 512, 1024, 2048.
 */
class EmptyMemoryBlock {
 public:
  //*** Constructor

  /**
   * @brief Construct a new EmptyMemoryBlock object, set the width of the empty
   * memory block
   *
   * @param[in] width the width of this type of empty memory block
   */
  explicit EmptyMemoryBlock(int width) { m_width = width; }

 public:
  //*** Public Functions of EmptyMemoryBlock Objects

  /**
   * @brief Allocate a block of empty memory. If the set of memory blocks of
   * size m_width has empty blocks available for allocation, which means there
   * are still elements in m_block, then return the empty memory block index
   * with the smallest index among all the empty blocks. If there are no empty
   * blocks, allocation fails, and this function returns -1.
   *
   * @return int: if allocation is successful, return the smallest element in
   * m_block, otherwise return -1.
   * @retval -1 allocation fails
   */
  int Allocate() {
    // Case 1: if the set is empty, allocation fails
    if (m_block.empty()) {
      return -1;
    }
    // Case 2: allocation succeeds, return the smallest element of m_block and
    // erase this block from the empty set
    int res = *m_block.begin();
    m_block.erase(m_block.begin());
    return res;
  }

  /**
   * @brief add the corresponding empty blocks (insert the left index of the
   * block into the m_block set)
   *
   * @param[in] idx the index of blocks
   * @param[in] size the size of blocks
   * @return int: the size of the empty block after this action
   */
  int AddBlock(int idx, int size) {
    if (size < m_width) return -1;
    int newIdx = idx + size - m_width;
    m_block.insert(newIdx);
    return size - m_width;
  }

  /**
   * @brief check whether the memory block with the beginning index idx is
   * empty, return the check result
   *
   * @param[in] idx the beginning index of this block
   * @retval true this block is empty
   * @retval false this block is not empty and has been allocated
   */
  bool IsEmpty(int idx) {
    std::set<int>::iterator it = m_block.find(idx);
    if (it != m_block.end())
      return true;
    else
      return false;
  }

 public:
  //*** Public Data Members of EmptyMemoryBlock Objects

  /**
   * @brief used to store the beginning indexes of all empty memory blocks with
   * m_width
   */
  std::set<int> m_block;

  /**
   * @brief the width of this empty memory block
   */
  int m_width;
};

#endif  // MEMORYLAYOUT_EMPTY_BLOCK_H_
