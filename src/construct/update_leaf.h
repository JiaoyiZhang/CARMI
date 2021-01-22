#ifndef UPDATE_LEAF_H
#define UPDATE_LEAF_H

#include <map>
#include "../baseNode.h"
using namespace std;

extern vector<BaseNode> entireChild;
extern map<double, int> scanLeaf;

void UpdateLeaf()
{
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