#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/histogram_node.h"

#include "./dataset/lognormal_distribution.h"
#include "./dataset/uniform_distribution.h"
#include "reconstruction.h"

#include <algorithm>
#include <random>
// #include <windows.h>
#include <time.h>
#include <iostream>
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"
#include "inner_noded_creator.h"
#include "leaf_node_creator.h"

using namespace std;

int datasetSize = 1000000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

btree::btree_map<double, double> btreemap;

LRNode lr;
NetworkNode nn;
HistogramNode histogram;
BinarySearchNode bin;

AdaptiveLR ada_lr;
AdaptiveNN ada_nn;
AdaptiveHis ada_his;
AdaptiveBin ada_bin;

int kLeafNodeID = 1;
int kInnerNodeID = 0;
int kNeuronNumber = 8;

int childNum = 25;
int kThreshold = 100000;
int kMaxKeyNum = 200000;

void btree_test(double &time0, double &time1, double &time2, double &time3)
{
    timespec t1, t2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < dataset.size(); i++)
        btreemap.find(dataset[i].first);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time0 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0); // ms
    cout << "Find time:" << time0 << endl;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.insert(insertDataset[i]);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time1 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "Insert time:" << time1 << endl;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.find(insertDataset[i].first);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time2 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "Update time:" << time2 << endl;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < insertDataset.size(); i++)
        btreemap.erase(insertDataset[i].first);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time3 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "Delete time:" << time3 << endl;
    cout << endl;
}

template <typename type>
void test(type obj, double &time0, double &time1, double &time2, double &time3)
{
    timespec t1, t2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < dataset.size(); i++)
        obj.Find(dataset[i].first);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time0 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "Find time:" << time0 << endl;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    for (int i = 0; i < insertDataset.size(); i++)
        obj.Insert(insertDataset[i]);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    time1 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    cout << "Insert time:" << time1 << endl;

    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     obj.Update({insertDataset[i].first, 1.11});
    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    // time2 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    // cout << "Update time:" << time1 << endl;

    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);
    // for (int i = 0; i < insertDataset.size(); i++)
    //     obj.Delete(insertDataset[i].first);
    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t2);
    // time3 += ((t2.tv_sec - t1.tv_sec)*1000.0 + float(t2.tv_nsec - t1.tv_nsec)/1000000.0);
    // cout << "Delete time:" << time1 << endl;
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

void printStructure()
{
    cout<<"print structure:"<<endl;
    ada_lr.PrintStructure();
    cout<<"//////////////////////////"<<endl;
    ada_nn.PrintStructure();
    cout<<"//////////////////////////"<<endl;
    ada_his.PrintStructure();
    cout<<"//////////////////////////"<<endl;
    ada_bin.PrintStructure();
    cout<<"//////////////////////////"<<endl;
}

void totalTest(bool isStatic, int repetitions)
{
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double lr_time0 = 0.0, lr_time1 = 0.0, lr_time2 = 0.0, lr_time3 = 0.0;
    double nn_time0 = 0.0, nn_time1 = 0.0, nn_time2 = 0.0, nn_time3 = 0.0;
    double his_time0 = 0.0, his_time1 = 0.0, his_time2 = 0.0, his_time3 = 0.0;
    double bin_time0 = 0.0, bin_time1 = 0.0, bin_time2 = 0.0, bin_time3 = 0.0;
    for (int i = 0; i < repetitions; i++)
    {
        btree::btree_map<double, double> btree;
        for (int l = 0; l < dataset.size(); l++)
            btree.insert(dataset[l]);
        btreemap = btree;
        cout << "btree:    " << i << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        printResult((i + 1), btree_time0, btree_time1, btree_time2, btree_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 0;
        if (isStatic)
        {
            lr = LRNode(childNum);
            lr.Initialize(dataset);
        }
        else
        {
            ada_lr = AdaptiveLR(childNum);
            ada_lr.Initialize(dataset);
        }
        cout << "lr init over!" << endl;
        cout << "****************" << endl;

        cout << "lr:    " << i << endl;
        if (isStatic)
            test(lr, lr_time0, lr_time1, lr_time2, lr_time3);
        else
            test(ada_lr, lr_time0, lr_time1, lr_time2, lr_time3);
        cout << endl;
        printResult((i + 1), lr_time0, lr_time1, lr_time2, lr_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 1;
        if (isStatic)
        {
            nn = NetworkNode(childNum);
            nn.Initialize(dataset);
        }
        else
        {
            ada_nn = AdaptiveNN(childNum);
            ada_nn.Initialize(dataset);
        }
        cout << "nn init over!" << endl;
        cout << "****************" << endl;

        cout << "nn:    " << i << endl;
        if (isStatic)
            test(nn, nn_time0, nn_time1, nn_time2, nn_time3);
        else
            test(ada_nn, nn_time0, nn_time1, nn_time2, nn_time3);
        cout << endl;
        printResult((i + 1), nn_time0, nn_time1, nn_time2, nn_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 2;
        if (isStatic)
        {
            histogram = HistogramNode(childNum);
            histogram.Initialize(dataset);
        }
        else
        {
            ada_his = AdaptiveHis(childNum);
            ada_his.Initialize(dataset);
        }
        cout << "his init over!" << endl;
        cout << "****************" << endl;
        cout << "his:    " << i << endl;
        if (isStatic)
            test(histogram, his_time0, his_time1, his_time2, his_time3);
        else
            test(ada_his, his_time0, his_time1, his_time2, his_time3);
        cout << endl;
        printResult((i + 1), his_time0, his_time1, his_time2, his_time3);
        cout << "-------------------------------" << endl;

        kInnerNodeID = 3;
        if (isStatic)
        {
            bin = BinarySearchNode(childNum);
            bin.Initialize(dataset);
        }
        else
        {
            ada_bin = AdaptiveBin(childNum);
            ada_bin.Initialize(dataset);
        }
        cout << "bin init over!" << endl;
        cout << "****************" << endl;

        cout << "bin:    " << i << endl;
        if (isStatic)
            test(bin, bin_time0, bin_time1, bin_time2, bin_time3);
        else
            test(ada_bin, bin_time0, bin_time1, bin_time2, bin_time3);
        cout << endl;
        printResult((i + 1), bin_time0, bin_time1, bin_time2, bin_time3);
        cout << "-------------------------------" << endl;
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "lr:" << endl;
    printResult(repetitions, lr_time0, lr_time1, lr_time2, lr_time3);

    cout << "nn:" << endl;
    printResult(repetitions, nn_time0, nn_time1, nn_time2, nn_time3);

    cout << "his:" << endl;
    printResult(repetitions, his_time0, his_time1, his_time2, his_time3);

    cout << "bin:" << endl;
    printResult(repetitions, bin_time0, bin_time1, bin_time2, bin_time3);

    // printStructure();
}

void testReconstructure(int testID, int repetitions)
{
    vector<pair<double, double>> totalData;
    for (int i = 0; i < dataset.size(); i++)
        totalData.push_back(dataset[i]);
    for (int i = 0; i < insertDataset.size(); i++)
        totalData.push_back(insertDataset[i]);
    std::sort(totalData.begin(), totalData.end(), [](pair<double, double> p1, pair<double, double> p2) {
        return p1.first < p2.first;
    });
    vector<pair<int, int>> cnt;
    int r=0,w=0;
    if(testID == 0)
    {
        // read:write = 1:10 (reconstructed from adaptive ga)
        for (int i = 0; i < totalData.size(); i++)
        {
            if ((i + 1) % 10 == 0)
            {
                cnt.push_back({1, 1});
                r+=1;
                w+=1;
            }
            else
            {
                cnt.push_back({0, 1});
                w+=1;
            }
        }
    }
    else if(testID == 1)
    {
        // almost all write
        for (int i = 0; i < totalData.size(); i++)
        {
            if ((i + 1) % 10000 == 0)
            {
                cnt.push_back({1, 1});
                r+=1;
                w+=1;
            }
            else
            {
                cnt.push_back({0, 1});
                w+=1;
            }
        }
    }
    else
    {
        for(int i = 0; i < 100000; i++)
        {
            cnt.push_back({0, 1});
            w+=1;
        }
        for(int i = 100000; i < totalData.size(); i++)
        {
            cnt.push_back({1, 0});
            r+=1;
        }
        
    }
    cout<<"total read times:"<<r<<endl;
    cout<<"total write times:"<<w<<endl;
    BasicInnerNode newRoot = reconstruction(totalData, cnt, dataset);

    double root_time0 = 0.0, root_time1 = 0.0, root_time2 = 0.0, root_time3 = 0.0;
    test(newRoot, root_time0, root_time1, root_time2, root_time3);
    cout << endl;
    printResult(1, root_time0, root_time1, root_time2, root_time3);
    cout << "-------------------------------" << endl;
}


int main()
{
    int repetitions = 1;
    cout<<"kLeafNodeID:"<<kLeafNodeID<<"\tleafNodeType:"<<typeid(LEAF_NODE_TYPE).name()<<endl;
    // cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
    LognormalDataset logData = LognormalDataset(datasetSize, 0.9);
    // logData.GenerateDataset(dataset, insertDataset);
    // cout << "-------------Static inner nodes----------------" << endl;
    // totalTest(true, repetitions);

    // dataset = vector<pair<double, double>>();
    // insertDataset = vector<pair<double, double>>();
    // cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    UniformDataset uniData = UniformDataset(datasetSize, 0.9);
    uniData.GenerateDataset(dataset, insertDataset);
    testReconstructure(0, repetitions);
    // cout << "-------------Static inner nodes----------------" << endl;
    // totalTest(true, repetitions);

    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    cout << "-------------Adaptive inner nodes----------------" << endl;
    totalTest(false, repetitions);

    cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++" << endl;
    // logData = LognormalDataset(datasetSize, 0.9);
    logData.GenerateDataset(dataset, insertDataset);
    testReconstructure(0, repetitions);
    cout << "-------------Adaptive inner nodes----------------" << endl;
    totalTest(false, repetitions);
    return 0;
}