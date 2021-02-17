#ifndef CHILD_ARRAY_H
#define CHILD_ARRAY_H
#include <iostream>
#include <vector>
#include "../carmi.h"
using namespace std;

// initialize entireChild
void CARMI::initEntireChild(int size)
{
    unsigned int len = 4096;
    while (len < size)
        len *= 2;
    len *= 2;
    entireChildNumber = len;
    entireChild.clear();
    nowChildNumber = 0;
    BaseNode tmp;
    for (int i = 0; i < len; i++)
        entireChild.push_back(tmp);
}

// allocate a block to the current inner node
// size: the size of the inner node needs to be allocated
// return the starting position of the allocation
// return -1, if it fails
int CARMI::allocateChildMemory(int size)
{
    int newLeft = -1;
    if (nowChildNumber + size <= entireChildNumber)
    {
        newLeft = nowChildNumber;
        nowChildNumber += size;
    }
    else
    {
        cout << "need expand the entireChild!" << endl;
        entireChildNumber *= 1.5;
        BaseNode t;
        for (int i = nowChildNumber; i < entireChildNumber; i++)
            entireChild.push_back(t);

        newLeft = nowChildNumber;
        nowChildNumber += size;
    }
    return newLeft;
}

#endif // !CHILD_ARRAY_H
