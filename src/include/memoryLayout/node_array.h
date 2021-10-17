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

#include "../baseNode.h"
/**
 * @brief the structure of node array
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class NodeArrayStructure {
 public:
  typedef std::pair<KeyType, ValueType> DataType;
  typedef std::vector<DataType> DataVectorType;
  NodeArrayStructure() {
    nowNodeNumber = 0;
    std::vector<BaseNode<KeyType, ValueType>>(4096,
                                              BaseNode<KeyType, ValueType>())
        .swap(nodeArray);
  }

  /**
   * @brief allocate a block of memory to store the child nodes of this node
   *
   * @param size the size of the block
   * @return int: the start index of the allocation
   * @retval -1 allocation fails
   */
  int AllocateNodeMemory(int size);

  /**
   * @brief release useless memory
   *
   * @param neededSize the size of needed memory
   */
  void ReleaseUselessMemory(int neededSize);

  std::vector<BaseNode<KeyType, ValueType>> nodeArray;  ///< the node array
  int nowNodeNumber;  ///< the number of inner nodes and leaf nodes
};

template <typename KeyType, typename ValueType>
int NodeArrayStructure<KeyType, ValueType>::AllocateNodeMemory(int size) {
  int newLeft = -1;
  unsigned int tmpSize = nodeArray.size();
  if (nowNodeNumber + size <= tmpSize) {
    newLeft = nowNodeNumber;
    nowNodeNumber += size;
  } else {
    BaseNode<KeyType, ValueType> t;
    while (nowNodeNumber + size > tmpSize) {
      tmpSize *= 2;
    }
    nodeArray.resize(tmpSize, t);

    newLeft = nowNodeNumber;
    nowNodeNumber += size;
  }
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
