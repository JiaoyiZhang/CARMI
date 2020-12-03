#ifndef EMPTY_BLOCK_H
#define EMPTY_BLOCK_H
#include <set>
using namespace std;

extern unsigned int entireDataSize;

class EmptyBlock
{
public:
    EmptyBlock(int w)
    {
        m_width = w;
        int len = entireDataSize / m_width;
        for (int i = 0; i < len; i++)
            m_block.insert(i * w);
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

    // delete the corresponding empty blocks
    bool release(int idx, int size)
    {
        if (size < m_width)
        {
            auto res = m_block.find(idx);
            if (res == m_block.end())
                return true;
            m_block.erase(res);
        }
        else
        {
            auto start = m_block.find(idx);
            auto len = size / m_width;
            for (int i = 0; i < len; i++, ++start)
                m_block.erase(start);
        }
        return true;
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