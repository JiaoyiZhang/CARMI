/**
 * @file empty_block.h
 * @author Jiaoyi
 * @brief the class of empty blocks
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_DATAMANAGER_EMPTY_BLOCK_H_
#define SRC_INCLUDE_DATAMANAGER_EMPTY_BLOCK_H_
#include <iostream>
#include <set>

class EmptyBlock {
 public:
  explicit EmptyBlock(int w) { m_width = w; }

  /**
   * @brief allocate a block and erase the corresponding key
   *
   * @param size the size that needs to be allocated
   * @return int return the key
   */
  int allocate(int size) {
    // if the set is empty, allocation fails
    if (m_block.empty()) return -1;
    auto res = *m_block.begin();
    m_block.erase(m_block.begin());
    return res;
  }

  /**
   * @brief add the corresponding empty blocks
   *
   * @param idx the index of blocks
   * @param size the size of blocks
   * @return int return the size of the empty block after this action
   */
  int addBlock(int idx, int size) {
    if (size < m_width) return -1;
    int newIdx = idx + size - m_width;
    m_block.insert(newIdx);
    return size - m_width;
  }

  /**
   * @brief find whether the block of this index is empty
   *
   * @param idx the index of this block
   * @return true this block is empty
   * @return false this block is not empty and has been allocated
   */
  bool find(int idx) {
    std::set<int>::iterator it = m_block.find(idx);
    if (it != m_block.end())
      return true;
    else
      return false;
  }

  std::set<int> m_block;
  int m_width;
};

#endif  // SRC_INCLUDE_DATAMANAGER_EMPTY_BLOCK_H_
