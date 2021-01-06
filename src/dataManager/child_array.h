#ifndef CHILD_ARRAY_H
#define CHILD_ARRAY_H
#include <iostream>
#include <vector>
#include "../baseNode.h"
using namespace std;

vector<BaseNode> entireChild;
unsigned int entireChildNumber;
unsigned int nowChildNumber;

// initialize entireChild
void initEntireChild(int size)
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
int allocateChildMemory(int size)
{
    int newLeft = -1;
    if (nowChildNumber + size <= entireChildNumber)
    {
        newLeft = nowChildNumber;
        nowChildNumber += size;
    }
    if (newLeft == -1)
    {
        cout << "need expand the entireChild!" << endl;
        unsigned int len = 4096;
        while (len < entireChildNumber)
            len *= 2;
        len *= 2;
        entireChildNumber = len;
        vector<BaseNode> tmp = entireChild;
        entireChild.clear();

        for (int i = 0; i < nowChildNumber; i++)
            entireChild.push_back(tmp[i]);
        BaseNode t;
        for (int i = nowChildNumber; i < entireChildNumber; i++)
            entireChild.push_back(t);

        // nowChildNumber = tmpEnd;
        newLeft = nowChildNumber;
        nowChildNumber += size;
    }
    return newLeft;
}

#endif // !CHILD_ARRAY_H
