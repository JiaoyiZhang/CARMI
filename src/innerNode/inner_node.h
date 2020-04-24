#ifndef INNER_NODE_H
#define INNER_NODE_H

#include <vector>
#include "../params.h"
#include "../leafNode/leaf_node.h"
#include "../../cpp-btree/btree_map.h"
using namespace std;

class BasicInnerNode
{
public:
    BasicInnerNode()
    {
        isLeafNode = false;
    }
    BasicInnerNode(int childNum)
    {
        isLeafNode = false;
        childNumber = childNum;
    }
    bool isLeaf() { return isLeafNode; }

    virtual void init(const vector<pair<double, double>> &dataset){};

    virtual pair<double, double> find(double key){};
    virtual bool insert(pair<double, double> data){};
    virtual bool del(double key){};
    virtual bool update(pair<double, double> data){};

protected:
    vector<BasicLeafNode *> children; // store the lower nodes
    int childNumber;                  // the size of the lower nodes
    bool isLeafNode;
};

#endif