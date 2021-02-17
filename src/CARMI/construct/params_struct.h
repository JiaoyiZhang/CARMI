#ifndef PARAMS_STRUCT_H
#define PARAMS_STRUCT_H
#include <vector>
using namespace std;

struct ParamStruct
{
    int type;       // 0-4
    int childNum;   // for inner nodes
    double density; // for leaf nodes
    // true: inner, false: leaf
    vector<pair<bool, pair<int, int>>> child; // the key in the corresponding map
};

#endif