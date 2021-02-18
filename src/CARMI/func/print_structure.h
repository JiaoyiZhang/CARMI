#ifndef PRINT_STRUCTURE_H
#define PRINT_STRUCTURE_H
#include <vector>
#include "../carmi.h"
using namespace std;

void CARMI::printStructure(vector<int> &levelVec, vector<int> &nodeVec, int level, int type, int idx)
{
    levelVec[level]++;
    vector<int> tree;
    for (int i = 0; i < 11; i++)
        tree.push_back(0);
    switch (type)
    {
    case 0:
    {
        cout << "level " << level << ": now root is lr, idx:" << idx << ", number:" << (root.lrRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.lrRoot.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        cout << "\tchild:";
        if (tree[4])
            cout << "\tlr:" << tree[4];
        if (tree[5])
            cout << "\tplr:" << tree[5];
        if (tree[6])
            cout << "\this:" << tree[6];
        if (tree[7])
            cout << "\tbin:" << tree[7];
        if (tree[8])
            cout << "\tarray:" << tree[8];
        if (tree[9])
            cout << "\tga:" << tree[9];
        if (tree[10])
            cout << "\tycsb:" << tree[10];
        cout << endl;
        for (int i = 0; i < (root.lrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.lrRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        cout << "level " << level << ": now root is plr, idx:" << idx << ", number:" << (root.plrRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (root.plrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.plrRoot.childLeft + i;
            int t = (entireChild[childIdx].plr.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        cout << "\tchild:";
        if (tree[4])
            cout << "\tlr:" << tree[4];
        if (tree[5])
            cout << "\tplr:" << tree[5];
        if (tree[6])
            cout << "\this:" << tree[6];
        if (tree[7])
            cout << "\tbin:" << tree[7];
        if (tree[8])
            cout << "\tarray:" << tree[8];
        if (tree[9])
            cout << "\tga:" << tree[9];
        if (tree[10])
            cout << "\tycsb:" << tree[10];
        cout << endl;
        for (int i = 0; i < (root.plrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.plrRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        cout << "level " << level << ": now root is his, idx:" << idx << ", number:" << (root.hisRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (root.hisRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.hisRoot.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        cout << "\tchild:";
        if (tree[4])
            cout << "\tlr:" << tree[4];
        if (tree[5])
            cout << "\tplr:" << tree[5];
        if (tree[6])
            cout << "\this:" << tree[6];
        if (tree[7])
            cout << "\tbin:" << tree[7];
        if (tree[8])
            cout << "\tarray:" << tree[8];
        if (tree[9])
            cout << "\tga:" << tree[9];
        if (tree[10])
            cout << "\tycsb:" << tree[10];
        cout << endl;
        for (int i = 0; i < (root.hisRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.hisRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        cout << "level " << level << ": now root is bin, idx:" << idx << ", number:" << (root.bsRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (root.bsRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.bsRoot.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        cout << "\tchild:";
        if (tree[4])
            cout << "\tlr:" << tree[4];
        if (tree[5])
            cout << "\tplr:" << tree[5];
        if (tree[6])
            cout << "\this:" << tree[6];
        if (tree[7])
            cout << "\tbin:" << tree[7];
        if (tree[8])
            cout << "\tarray:" << tree[8];
        if (tree[9])
            cout << "\tga:" << tree[9];
        if (tree[10])
            cout << "\tycsb:" << tree[10];
        cout << endl;
        for (int i = 0; i < (root.bsRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = root.bsRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }

    case 4:
    {
        // cout << "level " << level << ": now iplrer node is lr, idx:" << idx << ", number:" << (entireChild[idx].lr.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        // cout << "\tchild:";
        // if (tree[4])
        //     cout << "\tlr:" << tree[4];
        // if (tree[5])
        //     cout << "\tplr:" << tree[5];
        // if (tree[6])
        //     cout << "\this:" << tree[6];
        // if (tree[7])
        //     cout << "\tbin:" << tree[7];
        // if (tree[8])
        //     cout << "\tarray:" << tree[8];
        // if (tree[9])
        //     cout << "\tga:" << tree[9];
        // if (tree[10])
        //     cout << "\tycsb:" << tree[10];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 5:
    {
        // cout << "level " << level << ": now iplrer node is plr, idx:" << idx << ", number:" << (entireChild[idx].plr.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].plr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].plr.childLeft + i;
            int t = (entireChild[childIdx].plr.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        // cout << "\tchild:";
        // if (tree[4])
        //     cout << "\tlr:" << tree[4];
        // if (tree[5])
        //     cout << "\tplr:" << tree[5];
        // if (tree[6])
        //     cout << "\this:" << tree[6];
        // if (tree[7])
        //     cout << "\tbin:" << tree[7];
        // if (tree[8])
        //     cout << "\tarray:" << tree[8];
        // if (tree[9])
        //     cout << "\tga:" << tree[9];
        // if (tree[10])
        //     cout << "\tycsb:" << tree[10];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].plr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].plr.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 6:
    {
        // cout << "level " << level << ": now iplrer node is his, idx:" << idx << ", number:" << (entireChild[idx].his.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].plr.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        // cout << "\tchild:";
        // if (tree[4])
        //     cout << "\tlr:" << tree[4];
        // if (tree[5])
        //     cout << "\tplr:" << tree[5];
        // if (tree[6])
        //     cout << "\this:" << tree[6];
        // if (tree[7])
        //     cout << "\tbin:" << tree[7];
        // if (tree[8])
        //     cout << "\tarray:" << tree[8];
        // if (tree[9])
        //     cout << "\tga:" << tree[9];
        // if (tree[10])
        //     cout << "\tycsb:" << tree[10];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].his.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    case 7:
    {
        // cout << "level " << level << ": now iplrer node is bin, idx:" << idx << ", number:" << (entireChild[idx].bs.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
            nodeVec[t]++;
        }
        // cout << "\tchild:";
        // if (tree[4])
        //     cout << "\tlr:" << tree[4];
        // if (tree[5])
        //     cout << "\tplr:" << tree[5];
        // if (tree[6])
        //     cout << "\this:" << tree[6];
        // if (tree[7])
        //     cout << "\tbin:" << tree[7];
        // if (tree[8])
        //     cout << "\tarray:" << tree[8];
        // if (tree[9])
        //     cout << "\tga:" << tree[9];
        // if (tree[10])
        //     cout << "\tycsb:" << tree[10];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            // if (childIdx == idx)
            // {
            //     cout << " child wrong!" << endl;
            //     break;
            // }
            if (t > 3 && t < 8)
                printStructure(levelVec, nodeVec, level + 1, t, childIdx);
        }
        break;
    }
    }
}

#endif // !PRINT_STRUCTURE_H
