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

class DataRange
{
public:
    int left;
    int size;
    DataRange(int l, int s)
    {
        left = l;
        size = s;
    }
    DataRange(const DataRange &data)
    {
        left = data.left;
        size = data.size;
    }

    bool operator<(const DataRange &a) const
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
    DataRange initRange;
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
    int type;             // 0-4
    int childNum;         // for inner nodes
    double density;       // for leaf nodes
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

// change the name
class IndexPair
{
public:
    DataRange initRange;
    DataRange findRange;
    DataRange insertRange;
    IndexPair(DataRange init, DataRange find, DataRange insert) : initRange(init), findRange(find), insertRange(insert){};
};

class SubDataset
{
public:
    vector<DataRange> subInit;
    vector<DataRange> subFind;
    vector<DataRange> subInsert;

    SubDataset(int c) : subInit(vector<DataRange>(c, {-1, 0})), subFind(vector<DataRange>(c, {-1, 0})), subInsert(vector<DataRange>(c, {-1, 0})){};
    ~SubDataset(){};
};
#endif // !STRUCTURES_H