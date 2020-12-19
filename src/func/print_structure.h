#ifndef PRINT_STRUCTURE_H
#define PRINT_STRUCTURE_H
#include <vector>
#include "../innerNodeType/bin_type.h"
#include "../innerNodeType/his_type.h"
#include "../innerNodeType/lr_type.h"
#include "../innerNodeType/nn_type.h"
#include "../leafNodeType/ga_type.h"
#include "../leafNodeType/array_type.h"
using namespace std;

extern vector<LRType> LRVector;
extern vector<NNType> NNVector;
extern vector<HisType> HisVector;
extern vector<BSType> BSVector;
extern vector<ArrayType> ArrayVector;
extern vector<GappedArrayType> GAVector;


void printStructure(int level, int type, int idx)
{
    switch (type)
    {
    case 0:
    {
        cout << "level " << level << ": now root is lr, idx:" << idx << ", childNumber:" << LRVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < LRVector[idx].childNumber; i++)
        {
            auto content = entireChild[LRVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < LRVector[idx].childNumber; i++)
        {
            auto content = entireChild[LRVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 1:
    {
        cout << "level " << level << ": now root is nn, idx:" << idx << ", childNumber:" << NNVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < NNVector[idx].childNumber; i++)
        {
            auto content = entireChild[NNVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < NNVector[idx].childNumber; i++)
        {
            auto content = entireChild[NNVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 2:
    {
        cout << "level " << level << ": now root is his, idx:" << idx << ", childNumber:" << HisVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < HisVector[idx].childNumber; i++)
        {
            auto content = entireChild[HisVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < HisVector[idx].childNumber; i++)
        {
            auto content = entireChild[HisVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    case 3:
    {
        cout << "level " << level << ": now root is bin, idx:" << idx << ", childNumber:" << BSVector[idx].childNumber;
        vector<int> tree = {0, 0, 0, 0, 0, 0};
        for (int i = 0; i < BSVector[idx].childNumber; i++)
        {
            auto content = entireChild[BSVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            tree[t]++;
        }
        cout << "\tchild:";
        if (tree[0])
            cout << "\tlr:" << tree[0];
        if (tree[1])
            cout << "\tnn:" << tree[1];
        if (tree[2])
            cout << "\this:" << tree[2];
        if (tree[3])
            cout << "\tbin:" << tree[3];
        if (tree[4])
            cout << "\tarray:" << tree[4];
        if (tree[5])
            cout << "\tga:" << tree[5];
        cout << endl;
        for (int i = 0; i < BSVector[idx].childNumber; i++)
        {
            auto content = entireChild[BSVector[idx].childLeft + i];
            auto t = content >> 28;
            auto childIdx = content & 0x0FFFFFFF;
            if (t < 4)
                printStructure(level + 1, t, childIdx);
        }
        break;
    }
    }
}

#endif // !PRINT_STRUCTURE_H
