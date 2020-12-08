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

// initialize entireData
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
    for (int i = 0; i < 5; i++)
        emptyBlocks.push_back(EmptyBlock(256 * pow(2, i)));
}

// allocate a block to the current leaf node
// size: the size of the leaf node needs to be allocated
// return the starting position of the allocation
// return -1, if it fails
int allocateMemory(int size)
{
    // if (size >= 512)
    // {
    //     cout << "size:" << size << endl;
    //     for (int i = 0; i < 5; i++)
    //     {
    //         cout << "emptyBlocks: " << i << endl;
    //         for (auto it = emptyBlocks[i].m_block.begin(); it != emptyBlocks[i].m_block.end(); it++)
    //         {
    //             cout << *it << "\t";
    //         }
    //         cout << endl;
    //     }
    //     cout << "-----------------------------------------" << endl;
    // }
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
        for (int i = 0; i < 5; i++)
            tmpBlocks.push_back(emptyBlocks[i]);

        initEntireData(tmpSize);
        for (int i = 0; i < tmpSize; i++)
            entireData[i] = tmpData[i];
        for (int i = 0; i < 5; i++)
        {
            emptyBlocks[i] = tmpBlocks[i];
            int start = tmpSize;
            while (start % emptyBlocks[i].m_width != 0)
                start++;
            for (int j = start; j < entireDataSize; j += emptyBlocks[i].m_width)
                emptyBlocks[i].m_block.insert(j);
        }
        newLeft = emptyBlocks[idx].allocate(size);
    }
    // release the space in other emptyBlocks
    for (int i = 0; i < 5; i++)
    {
        if (i != idx)
            emptyBlocks[i].release(newLeft, size);
    }
    return newLeft;
}

// release the specified space
void releaseMemory(int left, int size)
{
    int type = log(size / 256) / log(2);
    emptyBlocks[type].m_block.insert(left);
    for (int i = 0; i < type; i++)
    {
        // insert the empty blocks
        for (int j = left; j < size; j += emptyBlocks[i].m_width)
            emptyBlocks[i].m_block.insert(j);
    }
    for (int i = type + 1; i < 5; i++)
    {
        // check if there is a need to merge
        if (left % emptyBlocks[i].m_width != 0)
        {
            if (emptyBlocks[i - 1].find(left - emptyBlocks[i - 1].m_width))
                emptyBlocks[i].m_block.insert(left - emptyBlocks[i - 1].m_width);
            else
                break;
        }
        else
        {
            if (emptyBlocks[i - 1].find(left + emptyBlocks[i - 1].m_width))
                emptyBlocks[i].m_block.insert(left);
            else
                break;
        }
    }
}

#endif // !DATA_POINT_H