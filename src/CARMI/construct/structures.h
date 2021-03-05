#ifndef STRUCTURES_H
#define STRUCTURES_H

/**
 * structures for CARMI
 */

#include <vector>
using namespace std;

struct RootStruct
{
    int rootType;
    int rootChildNum;
    RootStruct(int t, int c)
    {
        rootType = t;
        rootChildNum = c;
    }
};

struct NodeCost
{
    double time;
    double space;
    double cost;
    bool isInnerNode;
    NodeCost(){};
    NodeCost(double t, double s, double c, bool i)
    {
        time = t;
        space = s;
        cost = c;
        isInnerNode = i;
    }
};

struct SingleDataRange
{
    int left;
    int size;
    SingleDataRange(){};
    SingleDataRange(int l, int s)
    {
        left = l;
        size = s;
    }
};

struct MapKey
{
    bool isInnerNode;
    SingleDataRange initRange;
};

struct ParamStruct
{
    int type;       // 0-4
    int childNum;   // for inner nodes
    double density; // for leaf nodes
    // true: inner, false: leaf
    vector<MapKey> child; // the key in the corresponding map
    ParamStruct(){};
    ParamStruct(int t, int c, int d, vector<MapKey> tmpChild)
    {
        type = t;
        childNum = c;
        density = d;
        child = tmpChild;
    }
};

struct DataRange
{
    SingleDataRange initRange;
    SingleDataRange findRange;
    SingleDataRange insertRange;
    DataRange(){};
    DataRange(int initL, int initS, int findL, int findS, int insertL, int insertS)
    {
        initRange = SingleDataRange(initL, initS);
        initRange = SingleDataRange(findL, findS);
        initRange = SingleDataRange(insertL, insertS);
    }
};

struct SubSingleDataset
{
    vector<int> subLeft;
    vector<int> subSize;
    SubSingleDataset(){};
    SubSingleDataset(int c)
    {
        subLeft = vector<int>(c, -1);
        subSize = vector<int>(c, 0);
    }
};

struct SubDataset
{
    SubSingleDataset subInit;
    SubSingleDataset subFind;
    SubSingleDataset subInsert;

    SubDataset(int c)
    {
        subInit = SubSingleDataset(c);
        subFind = SubSingleDataset(c);
        subInsert = SubSingleDataset(c);
    }
};
#endif // !STRUCTURES_H