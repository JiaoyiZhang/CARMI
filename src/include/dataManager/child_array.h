/**
 * @file child_array.h
 * @author Jiaoyi
 * @brief manage the entireChild array
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_INCLUDE_DATAMANAGER_CHILD_ARRAY_H_
#define SRC_INCLUDE_DATAMANAGER_CHILD_ARRAY_H_
#include <iostream>
#include <vector>

#include "../carmi.h"

/**
 * @brief initialize entireChild
 *
 * @param size the size of datasets
 */
template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::InitEntireChild(int size) {
  unsigned int len = 4096;
  while (len < size) len *= 2;
  len *= 2;
  entireChildNumber = len;
  entireChild.clear();
  nowChildNumber = 0;
  entireChild = std::vector<BaseNode>(len, BaseNode());
}

/**
 * @brief allocate a block to the current inner node
 *
 * @param size the size of the inner node needs to be allocated
 * @return int the starting position of the allocation, return -1, if it fails
 */
template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateChildMemory(int size) {
  int newLeft = -1;
  if (nowChildNumber + size <= entireChildNumber) {
    newLeft = nowChildNumber;
    nowChildNumber += size;
  } else {
    entireChildNumber *= 2;
    BaseNode t;
    for (int i = nowChildNumber; i < entireChildNumber; i++)
      entireChild.push_back(t);

    newLeft = nowChildNumber;
    nowChildNumber += size;
  }
  return newLeft;
}

#endif  // SRC_INCLUDE_DATAMANAGER_CHILD_ARRAY_H_
