#ifndef EMPTY_BLOCK_H
#define EMPTY_BLOCK_H
#include <set>
#include <iostream>
using namespace std;

class EmptyBlock
{
public:
    EmptyBlock(int w)
    {
        m_width = w;
    }

    // allocate a block, return idx
    // erase the corresponding key
    int allocate(int size)
    {
        // if the set is empty, allocation fails
        if (m_block.empty())
            return -1;
        auto res = *m_block.begin();
        m_block.erase(m_block.begin());
        return res;
    }

    // add the corresponding empty blocks
    // return the size of the empty block after this action
    int addBlock(int idx, int size)
    {
        if (size < m_width)
            return -1;
        int newIdx = idx + size - m_width;
        m_block.insert(newIdx);
        return size - m_width;
    }

    bool find(int idx)
    {
        if (m_block.find(idx) == m_block.end())
            return false;
        else
            return true;
    }

    set<int> m_block;
    int m_width;
};

#endif // !EMPTY_BLOCK_H