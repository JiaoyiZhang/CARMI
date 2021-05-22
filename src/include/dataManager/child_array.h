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

template <typename KeyType, typename ValueType>
void CARMI<KeyType, ValueType>::InitEntireChild(int size) {
  nowChildNumber = 0;
  std::vector<BaseNode>(4096, BaseNode()).swap(entireChild);
}

template <typename KeyType, typename ValueType>
int CARMI<KeyType, ValueType>::AllocateChildMemory(int size) {
  int newLeft = -1;
  int tmpSize = entireChild.size();
  if (nowChildNumber + size <= tmpSize) {
    newLeft = nowChildNumber;
    nowChildNumber += size;
  } else {
    BaseNode t;
    while (nowChildNumber + size > tmpSize) {
      tmpSize *= 2;
    }
    entireChild.resize(tmpSize, t);

    newLeft = nowChildNumber;
    nowChildNumber += size;
  }
  return newLeft;
}

#endif  // SRC_INCLUDE_DATAMANAGER_CHILD_ARRAY_H_
