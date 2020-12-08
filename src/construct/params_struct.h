#ifndef PARAMS_STRUCT_H
#define PARAMS_STRUCT_H
#include <vector>
using namespace std;

struct InnerParams
{
    int type;
    int childNum;
    vector<pair<int, int>> child; // 0/1:inner/leaf;    idx: the key in the corresponding map
};

struct LeafParams
{
    int type;
    double density;
};

#endif