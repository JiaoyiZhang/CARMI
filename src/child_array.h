#ifndef CHILD_ARRAY_H
#define CHILD_ARRAY_H
#include <iostream>
#include <vector>
using namespace std;
extern int *entireChild;
extern unsigned int entireChildNumber;
extern unsigned int nowChildNumber;

// initialize entireChild
void initEntireChild(int size)
{
    unsigned int len = 4096;
    while (len < size)
        len *= 2;
    len *= 2;
    entireChildNumber = len;
    delete[] entireChild;
    nowChildNumber = 0;
    entireChild = new int[len];
    for (int i = 0; i < len; i++)
        entireChild[i] = -1;
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
        auto tmpSize = entireChildNumber;
        auto tmpEnd = nowChildNumber;
        vector<int> tmpChild;
        for (int i = 0; i < tmpSize; i++)
            tmpChild.push_back(entireChild[i]);

        initEntireChild(tmpSize);
        for (int i = 0; i < tmpSize; i++)
            entireChild[i] = tmpChild[i];
        nowChildNumber = tmpEnd;
        newLeft = nowChildNumber;
    }
    return newLeft;
}

#endif // !CHILD_ARRAY_H
