/**
 * @file empty_block.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_DATAMANAGER_EMPTY_BLOCK_H_
#define SRC_CARMI_DATAMANAGER_EMPTY_BLOCK_H_
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

  // add the corresponding empty blocks
  // return the size of the empty block after this action
  int addBlock(int idx, int size) {
    if (size < m_width) return -1;
    int newIdx = idx + size - m_width;
    m_block.insert(newIdx);
    return size - m_width;
  }

  bool find(int idx) {
    if (m_block.find(idx) == m_block.end())
      return false;
    else
      return true;
  }

  std::set<int> m_block;
  int m_width;
};

#endif  // SRC_CARMI_DATAMANAGER_EMPTY_BLOCK_H_
