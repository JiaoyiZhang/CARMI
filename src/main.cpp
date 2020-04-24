#include "./trainModel/lr.h"
#include "./trainModel/nn.h"

#include "./innerNode/inner_node.h"
#include "./innerNode/nn_node.h"
#include "./innerNode/lr_node.h"
#include "./innerNode/binary_search.h"
#include "./innerNode/division_node.h"

#include "./leafNode/leaf_node.h"
#include "./leafNode/array.h"
#include "./leafNode/gapped_array.h"

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

LRNode<ArrayNode> lr_array;
LRNode<GappedArray> lr_ga;
NetworkNode<ArrayNode> nn_array;
NetworkNode<GappedArray> nn_ga;
DivisionNode<ArrayNode> div_array;
DivisionNode<GappedArray> div_ga;
BinarySearchNode<ArrayNode> bin_array;
BinarySearchNode<GappedArray> bin_ga;

AdaptiveLR<ArrayNode> ada_lr_array;
AdaptiveLR<GappedArray> ada_lr_ga;
AdaptiveNN<ArrayNode> ada_nn_array;
AdaptiveNN<GappedArray> ada_nn_ga;
AdaptiveDiv<ArrayNode> ada_div_array;
AdaptiveDiv<GappedArray> ada_div_ga;
AdaptiveBin<ArrayNode> ada_bin_array;
AdaptiveBin<GappedArray> ada_bin_ga;

void createModel()
{
    for (int i = 0; i < dataset.size(); i++)
        btreemap.insert(dataset[i]);

    // // lognormalDistribution params
    // params firstStageParams(0.001, 100000, 8, 0.0001, 0.666);
    // params secondStageParams(0.001, 100000, 8, 0.0001, 0.666);

    // uniformDistrubution params
    params firstStageParams(0.00001, 500, 8, 0.0001, 0.00001);
    params secondStageParams(0.0000001, 1, 10000, 8, 0.0, 0.0);

    lr_array = LRNode<ArrayNode>(firstStageParams, secondStageParams, 1000, 15, 800);
    lr_array.init(dataset);
    cout << "lr_array init over!" << endl;
    cout << "****************" << endl;

    lr_ga = LRNode<GappedArray>(firstStageParams, secondStageParams, 1000, 15, 800);
    lr_ga.init(dataset);
    cout << "lr_ga init over!" << endl;
    cout << "****************" << endl;

    nn_array = NetworkNode<ArrayNode>(firstStageParams, secondStageParams, 1000, 15, 800);
    nn_array.init(dataset);
    cout << "nn_array init over!" << endl;
    cout << "****************" << endl;

    nn_ga = NetworkNode<GappedArray>(firstStageParams, secondStageParams, 10000, 15, 800);
    nn_ga.init(dataset);
    cout << "nn_ga init over!" << endl;
    cout << "****************" << endl;

    div_array = DivisionNode<ArrayNode>(secondStageParams, 1000, 15, 800);
    div_array.init(dataset);
    cout << "div_array init over!" << endl;
    cout << "****************" << endl;

    div_ga = DivisionNode<GappedArray>(secondStageParams, 10000, 15, 800);
    div_ga.init(dataset);
    cout << "div_ga init over!" << endl;
    cout << "****************" << endl;

    bin_array = BinarySearchNode<ArrayNode>(secondStageParams, 1000, 15, 800);
    bin_array.init(dataset);
    cout << "bin_array init over!" << endl;
    cout << "****************" << endl;

    bin_ga = BinarySearchNode<GappedArray>(secondStageParams, 1000, 15, 800);
    bin_ga.init(dataset);
    cout << "bin_ga init over!" << endl;
    cout << "****************" << endl;
    /*
        // ARMI_normal = adaptiveRMI<normalNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 15, 800);
        ARMI_normal = adaptiveRMI<normalNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1500, 12, 800);
        ARMI_normal.initialize(dataset);
        cout << "ARMI_normal init over!" << endl;
        cout << "****************" << endl;

        // SCALE_normal = scaleModel<normalNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_normal = scaleModel<normalNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_normal.initialize(dataset);
        cout << "SCALE_normal init over!" << endl;
        cout << "****************" << endl;

        // SRMI_gapped = staticRMI<gappedNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 2000, 15, 800);
        SRMI_gapped = staticRMI<gappedNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 5000, 15, 4600);
        SRMI_gapped.train();
        cout << "SRMI_gapped init over!" << endl;
        cout << "****************" << endl;

        // ARMI_gapped = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 15, 800);
        ARMI_gapped = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 12, 800);
        ARMI_gapped.initialize(dataset);
        cout << "ARMI_gapped init over!" << endl;
        cout << "****************" << endl;

        // SCALE_gapped = scaleModel<gappedNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_gapped = scaleModel<gappedNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_gapped.initialize(dataset);
        cout << "SCALE_gapped init over!" << endl;
        cout << "****************" << endl;
    */
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
        auto res = obj.find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.insert(insertDataset[i]);
    }
    QueryPerformanceCounter(&e);
    time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Insert time:" << time1 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.update({insertDataset[i].first, 1.11});
    }
    QueryPerformanceCounter(&e);
    time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Update time:" << time2 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.del(insertDataset[i].first);
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
    uniData.generateDataset(dataset, insertDataset);

    // // generateDataset: lognormal dataset
    // lognormalDataset logData = lognormalDataset(datasetSize, 0.9);
    // logData.generateDataset(dataset, insertDataset);

    createModel();
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double lr_array_time0 = 0.0, lr_array_time1 = 0.0, lr_array_time2 = 0.0, lr_array_time3 = 0.0;
    double lr_ga_time0 = 0.0, lr_ga_time1 = 0.0, lr_ga_time2 = 0.0, lr_ga_time3 = 0.0;
    double nn_array_time0 = 0.0, nn_array_time1 = 0.0, nn_array_time2 = 0.0, nn_array_time3 = 0.0;
    double nn_ga_time0 = 0.0, nn_ga_time1 = 0.0, nn_ga_time2 = 0.0, nn_ga_time3 = 0.0;
    double div_array_time0 = 0.0, div_array_time1 = 0.0, div_array_time2 = 0.0, div_array_time3 = 0.0;
    double div_ga_time0 = 0.0, div_ga_time1 = 0.0, div_ga_time2 = 0.0, div_ga_time3 = 0.0;
    double bin_array_time0 = 0.0, bin_array_time1 = 0.0, bin_array_time2 = 0.0, bin_array_time3 = 0.0;
    double bin_ga_time0 = 0.0, bin_ga_time1 = 0.0, bin_ga_time2 = 0.0, bin_ga_time3 = 0.0;
    int repetitions = 1;
    for (int i = 0; i < repetitions; i++)
    {
        cout << "btree:    " << i << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        printResult((i + 1), btree_time0, btree_time1, btree_time2, btree_time3);
        cout << "-------------------------------" << endl;

        cout << "lr_array:    " << i << endl;
        test(lr_array, lr_array_time0, lr_array_time1, lr_array_time2, lr_array_time3);
        cout << endl;
        printResult((i + 1), lr_array_time0, lr_array_time1, lr_array_time2, lr_array_time3);
        cout << "-------------------------------" << endl;

        cout << "lr_ga:    " << i << endl;
        test(lr_ga, lr_ga_time0, lr_ga_time1, lr_ga_time2, lr_ga_time3);
        cout << endl;
        printResult((i + 1), lr_ga_time0, lr_ga_time1, lr_ga_time2, lr_ga_time3);
        cout << "-------------------------------" << endl;

        cout << "nn_array:    " << i << endl;
        test(nn_array, nn_array_time0, nn_array_time1, nn_array_time2, nn_array_time3);
        cout << endl;
        printResult((i + 1), nn_array_time0, nn_array_time1, nn_array_time2, nn_array_time3);
        cout << "-------------------------------" << endl;

        cout << "nn_ga:    " << i << endl;
        test(nn_ga, nn_ga_time0, nn_ga_time1, nn_ga_time2, nn_ga_time3);
        cout << endl;
        printResult((i + 1), nn_ga_time0, nn_ga_time1, nn_ga_time2, nn_ga_time3);
        cout << "-------------------------------" << endl;

        cout << "div_array:    " << i << endl;
        test(div_array, div_array_time0, div_array_time1, div_array_time2, div_array_time3);
        cout << endl;
        printResult((i + 1), div_array_time0, div_array_time1, div_array_time2, div_array_time3);
        cout << "-------------------------------" << endl;

        cout << "div_ga:    " << i << endl;
        test(div_ga, div_ga_time0, div_ga_time1, div_ga_time2, div_ga_time3);
        cout << endl;
        printResult((i + 1), div_ga_time0, div_ga_time1, div_ga_time2, div_ga_time3);
        cout << "-------------------------------" << endl;

        cout << "bin_array:    " << i << endl;
        test(bin_array, bin_array_time0, bin_array_time1, bin_array_time2, bin_array_time3);
        cout << endl;
        printResult((i + 1), bin_array_time0, bin_array_time1, bin_array_time2, bin_array_time3);
        cout << "-------------------------------" << endl;

        cout << "bin_ga:    " << i << endl;
        test(bin_ga, bin_ga_time0, bin_ga_time1, bin_ga_time2, bin_ga_time3);
        cout << endl;
        printResult((i + 1), bin_ga_time0, bin_ga_time1, bin_ga_time2, bin_ga_time3);
        cout << "-------------------------------" << endl;

        createModel();
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "lr_array:" << endl;
    printResult(repetitions, lr_array_time0, lr_array_time1, lr_array_time2, lr_array_time3);

    cout << "lr_ga:" << endl;
    printResult(repetitions, lr_ga_time0, lr_ga_time1, lr_ga_time2, lr_ga_time3);

    cout << "nn_array:" << endl;
    printResult(repetitions, nn_array_time0, nn_array_time1, nn_array_time2, nn_array_time3);

    cout << "nn_ga:" << endl;
    printResult(repetitions, nn_ga_time0, nn_ga_time1, nn_ga_time2, nn_ga_time3);

    cout << "div_array:" << endl;
    printResult(repetitions, div_array_time0, div_array_time1, div_array_time2, div_array_time3);

    cout << "div_ga:" << endl;
    printResult(repetitions, div_ga_time0, div_ga_time1, div_ga_time2, div_ga_time3);

    cout << "bin_array:" << endl;
    printResult(repetitions, bin_array_time0, bin_array_time1, bin_array_time2, bin_array_time3);

    cout << "bin_ga:" << endl;
    printResult(repetitions, bin_ga_time0, bin_ga_time1, bin_ga_time2, bin_ga_time3);

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