#ifndef NODE_H
#define NODE_H

#include <array>
#include <vector>
using namespace std;

class node
{
public:
    node(){};

    virtual void initialize(vector<pair<double, double>> &dataset){};
    virtual long double getCost(btree::btree_map<double, pair<int, int>> cntTree, int childNum, vector<pair<double, double>> &dataset){};
    virtual void change(const vector<pair<int, int>> &cnt){};

    virtual int getSize(){};
    virtual bool isLeaf(){};
    virtual void getDataset(vector<pair<double, double>> &dataset){};
    virtual void train(const vector<pair<double, double>> &subDataset){};

    virtual pair<double, double> find(double key){};
    virtual bool insert(pair<double, double> data){};
    virtual bool del(double key){};
    virtual bool update(pair<double, double> data){};
};

#endif