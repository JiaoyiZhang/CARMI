#include "./trainModel/lr.h"
#include "./trainModel/nn.h"

#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/division_node.h"


#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "reconstruction.h"

#include <algorithm>
#include <random>
#include <windows.h>
#include <iostream>

using namespace std;

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

btree::btree_map<double, double> btreemap;

LRNode lr;
NetworkNode nn;
DivisionNode division;
BinarySearchNode bin;

AdaptiveLR ada_lr;
AdaptiveNN ada_nn;
AdaptiveDiv ada_div;
AdaptiveBin ada_bin;

void *InnerNodeCreator(int innerNodeType, int maxKeyNum, int childNum, int capacity)
{
    void *newNode;
    switch (innerNodeType)
    {
    case 0:
        newNode = new AdaptiveLR(maxKeyNum, childNum, capacity);
        break;
    case 1:
        newNode = new AdaptiveNN(maxKeyNum, childNum, capacity);
        break;
    case 2:
        newNode = new AdaptiveDiv(maxKeyNum, childNum, capacity);
        break;
    case 3:
        newNode = new AdaptiveBin(maxKeyNum, childNum, capacity);
        break;
    }
    return newNode;
}

void *LeafNodeCreator(int leafNodeType, int maxKeyNum, int capacity)
{
    void *newNode;
    switch (leafNodeType)
    {
    case 0:
        newNode = new ArrayNode(maxKeyNum, capacity);
        break;
    case 1:
        newNode = new GappedArray(maxKeyNum, capacity);
        break;
    }
    return newNode;
}
void createModel()
{
    for (int i = 0; i < dataset.size(); i++)
        btreemap.insert(dataset[i]);

    lr = LRNode(5000, 15, 800);
    lr.Initialize(dataset);
    cout << "lr init over!" << endl;
    cout << "****************" << endl;

    nn = NetworkNode(10000, 15, 800);
    nn.Initialize(dataset);
    cout << "nn init over!" << endl;
    cout << "****************" << endl;

    division = DivisionNode(10000, 15, 800);
    division.Initialize(dataset);
    cout << "div init over!" << endl;
    cout << "****************" << endl;

    bin = BinarySearchNode(1000, 15, 800);
    bin.Initialize(dataset);
    cout << "bin init over!" << endl;
    cout << "****************" << endl;
}

void btree_test(double &time0, double &time1, double &time2, double &time3)
{
    LARGE_INTEGER s, e, c;
    QueryPerformanceFrequency(&c);
    QueryPerformanceCounter(&s);
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);
    QueryPerformanceCounter(&e);
    time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Insert time:" << time1 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.find(insertDataset[i].first);
    QueryPerformanceCounter(&e);
    time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Update time:" << time2 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.erase(insertDataset[i].first);
    QueryPerformanceCounter(&e);
    time3 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Delete time:" << time3 << endl;
    cout << endl;
}

template <typename type>
void test(type obj, double &time0, double &time1, double &time2, double &time3)
{
    LARGE_INTEGER s, e, c;
    QueryPerformanceCounter(&s);
    QueryPerformanceFrequency(&c);
    for (int i = 0; i < dataset.size(); i++)
    {
        obj.Find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.Insert(insertDataset[i]);
    }
    QueryPerformanceCounter(&e);
    time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Insert time:" << time1 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.Update({insertDataset[i].first, 1.11});
    }
    QueryPerformanceCounter(&e);
    time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Update time:" << time2 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.Delete(insertDataset[i].first);
    }
    QueryPerformanceCounter(&e);
    time3 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Delete time:" << time3 << endl;
    cout << endl;
}

void printResult(int r, double &time0, double &time1, double &time2, double &time3)
{
    cout << "Average time: " << endl;
    cout << "Find time:" << time0 / (float)dataset.size() / float(r) << endl;
    cout << "Insert time:" << time1 / (float)insertDataset.size() / float(r) << endl;
    cout << "Update time:" << time2 / (float)insertDataset.size() / float(r) << endl;
    cout << "Delete time:" << time3 / (float)insertDataset.size() / float(r) << endl;
    cout << "***********************" << endl;
}
int main()
{
    // generateDataset: uniform dataset
    UniformDataset uniData = UniformDataset(datasetSize, 0.9);
    uniData.GenerateDataset(dataset, insertDataset);

    // // generateDataset: lognormal dataset
    // lognormalDataset logData = lognormalDataset(datasetSize, 0.9);
    // logData.generateDataset(dataset, insertDataset);

    createModel();
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double lr_time0 = 0.0, lr_time1 = 0.0, lr_time2 = 0.0, lr_time3 = 0.0;
    double nn_time0 = 0.0, nn_time1 = 0.0, nn_time2 = 0.0, nn_time3 = 0.0;
    double div_time0 = 0.0, div_time1 = 0.0, div_time2 = 0.0, div_time3 = 0.0;
    double bin_time0 = 0.0, bin_time1 = 0.0, bin_time2 = 0.0, bin_time3 = 0.0;
    int repetitions = 1;
    for (int i = 0; i < repetitions; i++)
    {
        cout << "btree:    " << i << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        printResult((i + 1), btree_time0, btree_time1, btree_time2, btree_time3);
        cout << "-------------------------------" << endl;

        cout << "lr:    " << i << endl;
        test(lr, lr_time0, lr_time1, lr_time2, lr_time3);
        cout << endl;
        printResult((i + 1), lr_time0, lr_time1, lr_time2, lr_time3);
        cout << "-------------------------------" << endl;

        cout << "nn:    " << i << endl;
        test(nn, nn_time0, nn_time1, nn_time2, nn_time3);
        cout << endl;
        printResult((i + 1), nn_time0, nn_time1, nn_time2, nn_time3);
        cout << "-------------------------------" << endl;

        cout << "div:    " << i << endl;
        test(division, div_time0, div_time1, div_time2, div_time3);
        cout << endl;
        printResult((i + 1), div_time0, div_time1, div_time2, div_time3);
        cout << "-------------------------------" << endl;

        cout << "bin:    " << i << endl;
        test(bin, bin_time0, bin_time1, bin_time2, bin_time3);
        cout << endl;
        printResult((i + 1), bin_time0, bin_time1, bin_time2, bin_time3);
        cout << "-------------------------------" << endl;

        createModel();
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "lr:" << endl;
    printResult(repetitions, lr_time0, lr_time1, lr_time2, lr_time3);

    cout << "nn:" << endl;
    printResult(repetitions, nn_time0, nn_time1, nn_time2, nn_time3);

    cout << "div:" << endl;
    printResult(repetitions, div_time0, div_time1, div_time2, div_time3);

    cout << "bin:" << endl;
    printResult(repetitions, bin_time0, bin_time1, bin_time2, bin_time3);

    // vector<pair<double, double>> totalData;
    // for (int i = 0; i < dataset.size(); i++)
    //     totalData.push_back(dataset[i]);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     totalData.push_back(insertDataset[i]);
    // std::sort(totalData.begin(), totalData.end(), [](pair<double, double> p1, pair<double, double> p2) {
    //     return p1.first < p2.first;
    // });
    // vector<pair<int, int>> cnt;
    // for (int i = 0; i < datasetSize; i++)
    // {
    //     if ((i + 1) % 10 == 0)
    //         cnt.push_back({0, 1});
    //     else
    //         cnt.push_back({1, 0});
    // }
    // reconstruction(totalData, cnt);

    return 0;
}