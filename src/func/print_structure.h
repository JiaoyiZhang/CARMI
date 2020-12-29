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

extern BaseNode **entireChild;

void printStructure(int level, int type, int idx)
{
    vector<int> tree;
    for (int i = 0; i < 71; i++)
        tree.push_back(0);
    switch (type)
    {
    case 0:
    {
        cout << "level " << level << ": now root is lr, idx:" << idx << ", childNumber:" << ((LRType *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((LRType *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((LRType *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((LRType *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((LRType *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        cout << "level " << level << ": now root is nn, idx:" << idx << ", childNumber:" << ((NNType *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((NNType *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((LRType *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((NNType *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((NNType *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        cout << "level " << level << ": now root is his, idx:" << idx << ", childNumber:" << ((HisType *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((HisType *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((LRType *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((HisType *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((HisType *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        cout << "level " << level << ": now root is bin, idx:" << idx << ", childNumber:" << ((BSType *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((BSType *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((BSType *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((BSType *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((BSType *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }

    case 65:
    {
        cout << "level " << level << ": now root is lr inner, idx:" << idx << ", childNumber:" << ((LRModel *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((LRModel *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((LRModel *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((LRModel *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((LRModel *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 66:
    {
        cout << "level " << level << ": now root is nn inner, idx:" << idx << ", childNumber:" << ((NNModel *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((NNModel *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((NNModel *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((NNModel *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((NNModel *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 67:
    {
        cout << "level " << level << ": now root is his inner, idx:" << idx << ", childNumber:" << ((HisModel *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((HisModel *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((HisModel *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((HisModel *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((HisModel *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 68:
    {
        cout << "level " << level << ": now root is bin inner, idx:" << idx << ", childNumber:" << ((BSModel *)entireChild[idx])->childNumber;
        for (int i = 0; i < ((BSModel *)entireChild[idx])->childNumber; i++)
        {
            auto childIdx = ((BSModel *)entireChild[idx])->childLeft + i;
            int t = entireChild[childIdx]->flag;
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
        for (int i = 0; i < ((BSModel *)entireChild[idx])->childNumber; i++)
        {
            auto t = ((BSModel *)entireChild[idx])->childLeft + i;
            auto childIdx = t;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    }
}

#endif // !PRINT_STRUCTURE_H
