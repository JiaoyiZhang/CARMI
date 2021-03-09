#ifndef DATA_POINT_H
#define DATA_POINT_H
#include <iostream>
#include <float.h>
#include <vector>
#include <math.h>
#include "empty_block.h"
#include "../carmi.h"
using namespace std;

// allocate empty blocks into emptyBlocks[i]
// left: the beginning idx of empty blocks
// len: the length of the blocks
inline bool CARMI::allocateEmptyBlock(int left, int len)
{
    if (len == 0)
        return true;
    int res = 0;
    for (int i = 12; i >= 0; i--)
    {
        res = emptyBlocks[i].addBlock(left, len);
        while (res > 0)
        {
            len = res;
            res = emptyBlocks[i].addBlock(left, len);
        }
        if (res == 0)
            return true;
    }
    return false;
}

// find the corresponding index in emptyBlocks
// return idx
inline int CARMI::getIndex(int size)
{
#ifdef DEBUG
    if (size > 4096 || size < 1)
        cout << "size: " << size << ",\tsize > 4096 || size < 1, getIndex WRONG!" << endl;
#endif // DEBUG
    int j = 4096;
    for (int i = 12; i >= 0; i--, j /= 2)
    {
        if (size <= j && size > j / 2)
            return i;
    }
    return -1;
}

// initialize entireData
void CARMI::initEntireData(int left, int size, bool reinit)
{
    unsigned int len = 4096;
    while (len < size)
        len *= 2;
    len *= 2;
    entireDataSize = len;
#ifdef DEBUG
    cout << "dataset size:" << size << endl;
    cout << "the size of entireData is:" << len << endl;
#endif // DEBUG
    vector<EmptyBlock>().swap(emptyBlocks);
    vector<pair<double, double>>().swap(entireData);
    entireData = vector<pair<double, double>>(len, {DBL_MIN, DBL_MIN});
    for (int i = 0, j = 1; i < 13; i++, j *= 2)
        emptyBlocks.push_back(EmptyBlock(j));
    if (reinit)
        len = size;
    auto res = allocateEmptyBlock(left, len);
#ifdef DEBUG
    if (!res)
        cout << "init allocateEmptyBlock WRONG!" << endl;
#endif // DEBUG
}

// allocate a block to the current leaf node
// size: the size of the leaf node needs to be allocated
// return the starting position of the allocation
// return -1, if it fails
int CARMI::allocateMemory(int size)
{
    int idx = getIndex(size); // idx in emptyBlocks[]
    size = emptyBlocks[idx].m_width;
#ifdef DEBUG
    if (idx == -1)
        cout << "getIndex in emptyBlocks WRONG!\tsize:" << size << endl;
#endif // DEBUG
    auto newLeft = -1;
    for (int i = idx; i < 13; i++)
    {
        newLeft = emptyBlocks[i].allocate(size);
        if (newLeft != -1)
        {
            idx = i;
            break;
        }
    }
    // allocation fails
    // need to expand the reorganize entireData
    if (newLeft == -1)
    {
#ifdef DEBUG
        cout << "need expand the entire!" << endl;
#endif // DEBUG
        unsigned int tmpSize = entireDataSize;
        vector<pair<double, double>> tmpData = entireData;
        vector<EmptyBlock> tmpBlocks = emptyBlocks;

        initEntireData(tmpSize, tmpSize, true);
        for (int i = 0; i < tmpSize; i++)
            entireData[i] = tmpData[i];
        for (int i = 0; i < 13; i++)
            emptyBlocks[i].m_block.insert(tmpBlocks[i].m_block.begin(), tmpBlocks[i].m_block.end());
        for (int i = idx; i < 13; i++)
        {
            newLeft = emptyBlocks[i].allocate(size);
            if (newLeft != -1)
            {
                idx = i;
                break;
            }
        }
    }

    // add the left blocks into the corresponding blocks
    auto res = allocateEmptyBlock(newLeft + size, emptyBlocks[idx].m_width - size);
#ifdef DEBUG
    if (!res)
        cout << "after allocate, allocateEmptyBlock WRONG!" << endl;
#endif // DEBUG
    if (newLeft + size > nowDataSize)
        nowDataSize = newLeft + size;
    return newLeft;
}

// release the specified space
void CARMI::releaseMemory(int left, int size)
{
    int idx = getIndex(size);
    for (int i = idx; i < 13; i++)
    {
        if (!emptyBlocks[i].find(left + emptyBlocks[i].m_width))
        {
            emptyBlocks[i].m_block.insert(left);
            break;
        }
    }
}

#endif // !DATA_POINT_H