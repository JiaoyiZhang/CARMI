// #ifndef RECONSTRUCTION_H
// #define RECONSTRUCTION_H

// #include "./trainModel/lr.h"
// #include "./trainModel/nn.h"

// #include "./innerNode/inner_node.h"
// #include "./innerNode/nn_node.h"
// #include "./innerNode/lr_node.h"
// #include "./innerNode/binary_search.h"
// #include "./innerNode/histogram_node.h"

// #include "./leafNode/leaf_node.h"
// #include "./leafNode/array.h"
// #include "./leafNode/gapped_array.h"

// #include "./dataset/lognormal_distribution.h"
// #include "./dataset/uniform_distribution.h"

// #include <algorithm>
// #include <iostream>
// #include <vector>
// using namespace std;

// long double CalculateCost(const btree::btree_map<double, pair<int, int>> &cntTree, int childNum, const vector<pair<double, double>> &data)
// {
//     long double tmpCost = BinarySearchNode::GetCost(cntTree, childNum, data);
//     return tmpCost;
// }

// BasicInnerNode reconstruction(const vector<pair<double, double>> &data, const vector<pair<int, int>> &cnt, const vector<pair<double, double>> &initData, int num)
// {
//     cout << endl;
//     cout << "-------------------------------" << endl;
//     cout << "Start reconstruction!" << endl;

//     int dataSize = data.size();
//     btree::btree_map<double, pair<int, int>> cntTree;
//     for (int i = 0; i < cnt.size(); i++)
//     {
//         cntTree.insert({data[i].first, cnt[i]}); // <key, <read, write>>
//     }

//     // Find the minimum childNum
//     long double minCost = 1e100;
//     int minNum = 10;
//     int maxChildNum = 300;
//     long double tmpCost;
//     cout<<"kRate:"<<kRate<<endl;
//     // LARGE_INTEGER s, e, c;
//     // QueryPerformanceFrequency(&c);
//     // QueryPerformanceCounter(&s);
//     for (int i = 2; i < maxChildNum; i++)
//     {
//         // cout << "calculate childNum :" << i << endl;
//         tmpCost = CalculateCost(cntTree, i, data);
//         // cout << "tmpCost is: " << tmpCost << "    minCost: " << minCost << "    minNum: " << minNum << endl;
//         if (tmpCost < minCost)
//         {
//             minCost = tmpCost;
//             minNum = i;
//         }
//     }
//     // QueryPerformanceCounter(&e);
//     // double time0 = (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
//     double time0 = 0;
//     cout << "find the minimum childNum: " << minNum << endl;
//     cout << "time:" << time0 << "ms" << endl;

//     // // Rebuild the tree according to the calculated most suitable childNum
//     BasicInnerNode root = BinarySearchNode(num);
//     root.Rebuild(data, initData, cntTree);
//     cout << "Rebuild root over!" << endl;
//     return root;
// }

// #endif