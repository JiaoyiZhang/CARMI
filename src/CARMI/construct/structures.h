#ifndef STRUCTURES_H
#define STRUCTURES_H

/**
 * structures for CARMI
 */

#include <vector>
#include <iostream>
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
};

class SingleDataRange
{
public:
    int left;
    int size;
    SingleDataRange(int l, int s)
    {
        left = l;
        size = s;
    }

    bool operator<(const SingleDataRange &a) const
    {
        if (left == a.left)
            return size < a.size;
        else
            return left < a.left;
    }
};

struct MapKey
{
    bool isInnerNode;
    SingleDataRange initRange;
    bool operator<(const MapKey &a) const
    {
        if (initRange.left == a.initRange.left)
            return initRange.size < a.initRange.size;
        else
            return initRange.left < a.initRange.left;
    }
};

struct ParamStruct
{
    int type;       // 0-4
    int childNum;   // for inner nodes
    double density; // for leaf nodes
    // true: inner, false: leaf
    vector<MapKey> child; // the key in the corresponding map
    ParamStruct(){};
    ParamStruct(int t, int c, double d, vector<MapKey> tmpChild)
    {
        type = t;
        childNum = c;
        density = d;
        child = tmpChild;
    }
};

class DataRange
{
public:
    SingleDataRange initRange;
    SingleDataRange findRange;
    SingleDataRange insertRange;
    DataRange(int initL, int initS, int findL, int findS, int insertL, int insertS) : initRange(initL, initS), findRange(findL, findS), insertRange(insertL, insertS){};
};

class SubSingleDataset
{
public:
    vector<int> subLeft;
    vector<int> subSize;

    SubSingleDataset(int c) : subLeft(c, -1), subSize(c, 0){};
    ~SubSingleDataset(){};
};

class SubDataset
{
public:
    SubSingleDataset *subInit;
    SubSingleDataset *subFind;
    SubSingleDataset *subInsert;

    SubDataset(int c) : subInit(new SubSingleDataset(c)), subFind(new SubSingleDataset(c)), subInsert(new SubSingleDataset(c)){};
    ~SubDataset(){};
};
#endif // !STRUCTURES_H