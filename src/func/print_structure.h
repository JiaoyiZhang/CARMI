#ifndef PRINT_STRUCTURE_H
#define PRINT_STRUCTURE_H
#include <vector>
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"

#include "../innerNode/bs_model.h"
#include "../innerNode/lr_model.h"
#include "../innerNode/nn_model.h"
#include "../innerNode/his_model.h"
using namespace std;

extern vector<BaseNode> entireChild;
extern vector<int> levelVec;

void printStructure(int level, int type, int idx)
{
    levelVec[level]++;
    vector<int> tree;
    for (int i = 0; i < 10; i++)
        tree.push_back(0);
    switch (type)
    {
    case 0:
    {
        // cout << "level " << level << ": now root is lr, idx:" << idx << ", number:" << (lrRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (lrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = lrRoot.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (lrRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = lrRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        // cout << "level " << level << ": now root is nn, idx:" << idx << ", number:" << (nnRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (nnRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = nnRoot.childLeft + i;
            int t = (entireChild[childIdx].nn.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (nnRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = nnRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        // cout << "level " << level << ": now root is his, idx:" << idx << ", number:" << (hisRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (hisRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = hisRoot.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (hisRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = hisRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        // cout << "level " << level << ": now root is bin, idx:" << idx << ", number:" << (bsRoot.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (bsRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = bsRoot.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (bsRoot.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = bsRoot.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }

    case 4:
    {
        // cout << "level " << level << ": now inner node is lr, idx:" << idx << ", number:" << (entireChild[idx].lr.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        if (tree[4])
            // cout << "\tlr:" << tree[4];
        if (tree[5])
            // cout << "\tnn:" << tree[5];
        if (tree[6])
            // cout << "\this:" << tree[6];
        if (tree[7])
            // cout << "\tbin:" << tree[7];
        if (tree[8])
            // cout << "\tarray:" << tree[8];
        if (tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 5:
    {
        // cout << "level " << level << ": now inner node is nn, idx:" << idx << ", number:" << (entireChild[idx].nn.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].nn.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].nn.childLeft + i;
            int t = (entireChild[childIdx].nn.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].nn.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].nn.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 6:
    {
        // cout << "level " << level << ": now inner node is his, idx:" << idx << ", number:" << (entireChild[idx].his.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].nn.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].his.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].his.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 7:
    {
        // cout << "level " << level << ": now inner node is bin, idx:" << idx << ", number:" << (entireChild[idx].bs.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
        }
        // cout << "\tchild:";
        // if(tree[4])
            // cout << "\tlr:" << tree[4];
        // if(tree[5])
            // cout << "\tnn:" << tree[5];
        // if(tree[6])
            // cout << "\this:" << tree[6];
        // if(tree[7])
            // cout << "\tbin:" << tree[7];
        // if(tree[8])
            // cout << "\tarray:" << tree[8];
        // if(tree[9])
            // cout << "\tga:" << tree[9];
        // cout << endl;
        for (int i = 0; i < (entireChild[idx].bs.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            auto t = (entireChild[childIdx].lr.flagNumber >> 24);
            if (t > 3 && t < 8)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    }
}

#endif // !PRINT_STRUCTURE_H
