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

void printStructure(int level, int type, int idx)
{
    vector<int> tree;
    for (int i = 0; i < 71; i++)
        tree.push_back(0);
    switch (type)
    {
    case 0:
    {
        cout << "level " << level << ": now root is lr, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].lr.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto t = entireChild[idx].lr.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        cout << "level " << level << ": now root is nn, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].nn.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].nn.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].nn.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].nn.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].nn.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        cout << "level " << level << ": now root is his, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].his.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].his.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].his.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].his.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        cout << "level " << level << ": now root is bin, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].bs.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].bs.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].bs.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].bs.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }

    case 4:
    {
        cout << "level " << level << ": now inner node is lr, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].lr.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].lr.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < (entireChild[idx].lr.flagNumber & 0x00FFFFFF); i++)
        {
            auto t = entireChild[idx].lr.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 5:
    {
        cout << "level " << level << ": now inner node is nn, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].nn.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].nn.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].nn.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].nn.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].nn.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 6:
    {
        cout << "level " << level << ": now inner node is his, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].his.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].his.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].lr.childLeft + i;
            int t = (entireChild[childIdx].his.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].his.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].his.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 7:
    {
        cout << "level " << level << ": now inner node is bin, idx:" << idx << ", flagNumber & 0x00FFFFFF:" << (entireChild[idx].bs.flagNumber & 0x00FFFFFF);
        for (int i = 0; i < entireChild[idx].bs.flagNumber & 0x00FFFFFF; i++)
        {
            auto childIdx = entireChild[idx].bs.childLeft + i;
            int t = (entireChild[childIdx].bs.flagNumber >> 24);
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[65])
            cout << "\tlr:" << tree[65];
        if (tree[66])
            cout << "\tnn:" << tree[66];
        if (tree[67])
            cout << "\this:" << tree[67];
        if (tree[68])
            cout << "\tbin:" << tree[68];
        if (tree[69])
            cout << "\tarray:" << tree[69];
        if (tree[70])
            cout << "\tga:" << tree[70];
        cout << endl;
        for (int i = 0; i < entireChild[idx].bs.flagNumber & 0x00FFFFFF; i++)
        {
            auto t = entireChild[idx].bs.childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    }
}

#endif // !PRINT_STRUCTURE_H
