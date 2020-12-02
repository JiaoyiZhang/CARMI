#ifndef DATA_POINT_H
#define DATA_POINT_H
#include <iostream>
#include <float.h>
#include <vector>
using namespace std;
extern pair<double, double> *entireData;
extern int *mark;
extern unsigned int entireDataSize;

// initialize entireData and mark
void initEntireData(int size)
{
    unsigned int len = 4096;
    while (len < size)
    {
        len *= 2;
    }
    len *= 2;
    entireDataSize = len;
    // cout << "dataset size:" << size << endl;
    // cout << "the size of entireData is:" << len << endl;
    delete[] entireData;
    delete[] mark;
    entireData = new pair<double, double>[len];
    for (int i = 0; i < len; i++)
        entireData[i] = {DBL_MIN, DBL_MIN};
    mark = new int[len / 16];
    // cout << "the size of mark is:" << len / 16 << endl;
    for (int i = 0; i < len / 16; i++)
        mark[i] = 0; // each block is unused
}

// allocate a block to the current leaf node
// size: the size of the leaf node needs to be allocated, must be a multiple of 16
// return the starting position of the allocation
// return -1, if it fails
int allocateMemory(int size)
{
    int m = size / 16; // need m blocks
    for (int i = 0; i < entireDataSize / 16; i++)
    {
        if (mark[i] == 0)
        {
            bool check = true;
            for (int j = i + 1; j < i + m; j++)
            {
                if ((j >= entireDataSize / 16) || (mark[j] == 1))
                {
                    check = false;
                    i = j;
                    break;
                }
            }
            if (check == true)
            {
                // allocate blocks
                for (int k = i; k < i + m; k++)
                    mark[k] = 1;
                return i * 16;
            }
        }
    }
    // space is not enough, array needs to be expanded
    cout << "need expand the entire!" << endl;
    auto tmpSize = entireDataSize;
    vector<pair<double, double>> tmpData;
    vector<int> tmpMark;
    for (int i = 0; i < tmpSize; i++)
        tmpData.push_back(entireData[i]);
    for (int i = 0; i < tmpSize / 16; i++)
        tmpMark.push_back(mark[i]);

    initEntireData(tmpSize);
    for (int i = 0; i < tmpSize; i++)
        entireData[i] = tmpData[i];
    for (int i = 0; i < tmpSize / 16; i++)
        mark[i] = tmpMark[i];
    for (int i = tmpSize / 16; i < tmpSize / 16 + m; i++)
        mark[i] = 1;
    return tmpSize;
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