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
#ifndef DATAMANAGER_NODE_ARRAY_H_
#define DATAMANAGER_NODE_ARRAY_H_
#include <iostream>
#include <utility>
#include <vector>

#include "../base_node.h"
/**
 * @brief the structure of node array
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class NodeArrayStructure {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief the pair of data points
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief the vector of data points, which is the type of dataset
   */
  typedef std::vector<DataType> DataVectorType;

  /**
   * @brief Construct a new Node Array Structure object
   * Initialize the nowNodeNumber with 0 and construct 4096 empty members of
   * BaseNode vector
   */
  NodeArrayStructure() {
    nowNodeNumber = 0;
    std::vector<BaseNode<KeyType, ValueType>>(4096,
                                              BaseNode<KeyType, ValueType>())
        .swap(nodeArray);
  }

 public:
  //*** Public Functions of NodeArrayStructure

  /**
   * @brief allocate a block of empty memory to store the nodes
   *
   * @param[in] size the size of nodes needed to be stored
   * @return int: the beginning index of this allocated memory
   * @retval -1 allocation fails
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
  std::vector<BaseNode<KeyType, ValueType>> nodeArray;

  /**
   * @brief the used size of nodeArray
   */
  int nowNodeNumber;
};

template <typename KeyType, typename ValueType>
int NodeArrayStructure<KeyType, ValueType>::AllocateNodeMemory(int size) {
  int newLeft = -1;
  unsigned int tmpSize = nodeArray.size();

  // allocation fails, need to expand the nodeArray
  while (nowNodeNumber + size > tmpSize) {
    BaseNode<KeyType, ValueType> t;
    tmpSize *= 2;
    nodeArray.resize(tmpSize, t);
  }
  newLeft = nowNodeNumber;
  nowNodeNumber += size;
  return newLeft;
}

template <typename KeyType, typename ValueType>
void NodeArrayStructure<KeyType, ValueType>::ReleaseUselessMemory(
    int neededSize) {
  std::vector<BaseNode<KeyType, ValueType>> tmp(nodeArray.begin(),
                                                nodeArray.begin() + neededSize);
  std::vector<BaseNode<KeyType, ValueType>>().swap(nodeArray);
  nodeArray = tmp;
}
#endif  // DATAMANAGER_NODE_ARRAY_H_
