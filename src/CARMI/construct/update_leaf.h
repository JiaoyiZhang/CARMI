#ifndef UPDATE_LEAF_H
#define UPDATE_LEAF_H

#include <map>
#include "../baseNode.h"
#include "../carmi.h"
using namespace std;

void CARMI::UpdateLeaf()
{
    if (kIsYCSB)
        return;
    auto it = scanLeaf.begin();
    int pre = it->second;
    auto next = it;
    next++;
    entireChild[it->second].array.nextLeaf = next->second;
    it++;

    for (; it != scanLeaf.end(); it++)
    {
        next = it;
        next++;
        if (next == scanLeaf.end())
        {
            entireChild[it->second].array.previousLeaf = pre;
        }
        else
        {
            entireChild[it->second].array.previousLeaf = pre;
            pre = it->second;
            entireChild[it->second].array.nextLeaf = next->second;
        }
    }
}

#endif // !UPDATE_LEAF_H