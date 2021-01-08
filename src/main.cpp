
#include <algorithm>
#include <random>
#include <iostream>
#include <algorithm>
#include <fstream>

#include "./experiment/construct_map.h"
#include "./experiment/construct_ycsb.h"
#include "./experiment/construct_synthetic.h"
#include "./experiment/test_synthetic.h"
using namespace std;

int datasetSize = 67108864;
// int datasetSize = 26214400;
// int datasetSize = 1000000;
int initDatasetSize;

vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

vector<pair<double, double>> findActualDataset;
vector<pair<double, double>> insertActualDataset;

int kLeafNodeID = 0;
int kInnerNodeID = 0;

ofstream outRes;

LRType lrRoot;
NNType nnRoot;
HisType hisRoot;
BSType bsRoot;

int main()
{
    outRes.open("res_0108.csv", ios::app);
    outRes << "\nTest time: " << __TIMESTAMP__ << endl;
    kMaxKeyNum = 2;
    // kLeafNodeID = 1;
    cout << "kLeafNodeID:" << kLeafNodeID << endl;
    // // static structure
    // testSynthetic(1);
    // testSynthetic(0.5);
    // testSynthetic(0.95);
    // testSynthetic(0);

    kMaxKeyNum = 512;
    constructSynthetic(1);   // read-only
    // constructSynthetic(0.5); // balance
    // constructSynthetic(0.95);
    // constructSynthetic(0); // partial

    constructYCSB(1);   // read-only
    // constructYCSB(0.5); // balance
    // constructYCSB(0.95);
    // constructYCSB(0); // partial

    // kMaxKeyNum = 256;

    constructMap(1);   // read-only
    // constructMap(0.5); // balance
    // constructMap(0.95);
    // constructMap(0); // partial

    outRes << "----------------------------------------------" << endl;

    return 0;
}