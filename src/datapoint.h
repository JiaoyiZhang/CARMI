#ifndef DATA_POINT_H
#define DATA_POINT_H
#include <iostream>
#include <float.h>
#include <vector>
#include <math.h>
#include "empty_block.h"
using namespace std;
extern pair<double, double> *entireData;
extern unsigned int entireDataSize;
extern vector<EmptyBlock> emptyBlocks;

// initialize entireData and mark
void initEntireData(int size)
{
    unsigned int len = 4096;
    while (len < size)
        len *= 2;
    len *= 2;
    entireDataSize = len;
    // cout << "dataset size:" << size << endl;
    // cout << "the size of entireData is:" << len << endl;
    delete[] entireData;
    vector<EmptyBlock>().swap(emptyBlocks);
    entireData = new pair<double, double>[len];
    for (int i = 0; i < len; i++)
        entireData[i] = {DBL_MIN, DBL_MIN};
    for (int i = 0; i < 4; i++)
        emptyBlocks.push_back(EmptyBlock(256 * pow(2, i)));
}

// allocate a block to the current leaf node
// size: the size of the leaf node needs to be allocated, must be a multiple of 16
// return the starting position of the allocation
// return -1, if it fails
int allocateMemory(int size)
{
    int idx = log(size / 256) / log(2); // idx in emptyBlocks[]
    auto newLeft = emptyBlocks[idx].allocate(size);
    if (newLeft == -1)
    {
        // allocation fails
        // need to expand the reorganize entireData
        cout << "need expand the entire!" << endl;
        auto tmpSize = entireDataSize;
        vector<pair<double, double>> tmpData;
        vector<EmptyBlock> tmpBlocks;
        for (int i = 0; i < tmpSize; i++)
            tmpData.push_back(entireData[i]);
        for (int i = 0; i < 4; i++)
            tmpBlocks.push_back(emptyBlocks[i]);

        initEntireData(tmpSize);
        for (int i = 0; i < tmpSize; i++)
            entireData[i] = tmpData[i];
        for (int i = 0; i < 4; i++)
        {
            emptyBlocks[i] = tmpBlocks[i];
            for (int j = tmpSize; j < tmpSize / 16 + m; j++)
                mark[i] = 1;
        }
    }
    // release the space in other emptyBlocks
    for (int i = 0; i < 4; i++)
    {
        if (i != idx)
            emptyBlocks[i].release(idx, size);
    }
    return newLeft;
}

// release the specified space
void releaseMemory(int left, int size)
{
    left /= 16;
    size /= 16;
    for (int i = left; i < left + size; i++)
        mark[i] = 0;
}

#endif // !DATA_POINT_H