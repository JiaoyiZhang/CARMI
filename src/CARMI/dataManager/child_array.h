/**
 * @file child_array.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_CARMI_DATAMANAGER_CHILD_ARRAY_H_
#define SRC_CARMI_DATAMANAGER_CHILD_ARRAY_H_
#include <iostream>
#include <vector>

#include "../carmi.h"

// initialize entireChild
void CARMI::initEntireChild(int size) {
  unsigned int len = 4096;
  while (len < size) len *= 2;
  len *= 2;
  entireChildNumber = len;
  entireChild.clear();
  nowChildNumber = 0;
  entireChild = vector<BaseNode>(len, BaseNode());
}

// allocate a block to the current inner node
// size: the size of the inner node needs to be allocated
// return the starting position of the allocation
// return -1, if it fails
int CARMI::allocateChildMemory(int size) {
  int newLeft = -1;
  if (nowChildNumber + size <= entireChildNumber) {
    newLeft = nowChildNumber;
    nowChildNumber += size;
  } else {
#ifdef DEBUG
    cout << "need expand the entireChild!" << endl;
#endif  // DEBUG
    entireChildNumber *= 1.5;
    BaseNode t;
    for (int i = nowChildNumber; i < entireChildNumber; i++)
      entireChild.push_back(t);

    newLeft = nowChildNumber;
    nowChildNumber += size;
  }
  return newLeft;
}

#endif  // SRC_CARMI_DATAMANAGER_CHILD_ARRAY_H_
