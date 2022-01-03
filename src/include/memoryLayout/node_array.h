/**
 * @file node_array.h
 * @author Jiaoyi
 * @brief manage the node array
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef MEMORYLAYOUT_NODE_ARRAY_H_
#define MEMORYLAYOUT_NODE_ARRAY_H_
#include <functional>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

#include "../base_node.h"

/**
 * @brief the structure of node array
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 * @tparam Compare A binary predicate that takes two element keys as arguments
 * and returns a bool.
 * @tparam Alloc Type of the allocator object used to define the storage
 * allocation model.
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
class NodeArrayStructure {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief Construct a new Node Array Structure object
   * Initialize the nowNodeNumber with 0 and construct 4096 empty members of
   * BaseNode vector
   */
  NodeArrayStructure() {
    nowNodeNumber = 0;
    std::vector<BaseNode<KeyType, ValueType, Compare, Alloc>>(
        4096, BaseNode<KeyType, ValueType, Compare, Alloc>())
        .swap(nodeArray);
  }

 public:
  //*** Public Functions of NodeArrayStructure

  /**
   * @brief allocate a block of empty memory to store the nodes
   *
   * @param[in] size the size of nodes needed to be stored
   * @return int: the beginning index of this allocated memory
   */
  int AllocateNodeMemory(int size);

  /**
   * @brief After the construction of CARMI is completed, the useless memory
   * exceeding the needed size will be released.
   *
   * @param[in] neededSize the size of needed node blocks
   */
  void ReleaseUselessMemory(int neededSize);

 public:
  //*** Public Data Member of Node Array Structure Objects

  /**
   * @brief the node array mentioned in the paper.
   *
   * All tree nodes, including both inner nodes and leaf nodes, are stored in
   * this node array. Each member occupies a fixed size according to the
   * BaseNode class.
   */
  std::vector<BaseNode<KeyType, ValueType, Compare, Alloc>> nodeArray;

  /**
   * @brief the used size of nodeArray
   */
  int nowNodeNumber;
};

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
int NodeArrayStructure<KeyType, ValueType, Compare, Alloc>::AllocateNodeMemory(
    int size) {
  if (size < 0) {
    throw std::invalid_argument(
        "NodeArrayStructure::AllocateNodeMemory: the size is less than 0.");
  }
  int newLeft = -1;
  unsigned int tmpSize = nodeArray.size();

  // allocation fails, need to expand the nodeArray
  while (nowNodeNumber + size > tmpSize) {
    BaseNode<KeyType, ValueType, Compare, Alloc> t;
    tmpSize *= 1.25;
    nodeArray.resize(tmpSize, t);
  }
  newLeft = nowNodeNumber;
  nowNodeNumber += size;
  return newLeft;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
void NodeArrayStructure<KeyType, ValueType, Compare,
                        Alloc>::ReleaseUselessMemory(int neededSize) {
  if (neededSize < 0) {
    throw std::invalid_argument(
        "NodeArrayStructure::ReleaseUselessMemory: the size is less than 0.");
  }
  std::vector<BaseNode<KeyType, ValueType, Compare, Alloc>> tmp(
      nodeArray.begin(), nodeArray.begin() + neededSize);
  std::vector<BaseNode<KeyType, ValueType, Compare, Alloc>>().swap(nodeArray);
  nodeArray = tmp;
}
#endif  // MEMORYLAYOUT_NODE_ARRAY_H_
