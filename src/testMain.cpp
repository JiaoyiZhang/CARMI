#include "./trainModel/lr.h"
#include "./trainModel/nn.h"
#include "./innerNode/adaptiveRMI.h"
#include "./innerNode/staticRMI.h"
#include "./innerNode/scaleModel.h"
#include "./leafNode/gappedNode.h"
#include "./leafNode/normalNode.h"
#include "./dataset/lognormalDistribution.h"
#include "./dataset/uniformDistribution.h"
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

staticRMI<normalNode<linearRegression>, linearRegression> SRMI_normal;
adaptiveRMI<normalNode<linearRegression>, linearRegression> ARMI_normal;
scaleModel<normalNode<linearRegression>> SCALE_normal;
staticRMI<gappedNode<linearRegression>, linearRegression> SRMI_gapped;
adaptiveRMI<gappedNode<linearRegression>, linearRegression> ARMI_gapped;
scaleModel<gappedNode<linearRegression>> SCALE_gapped;

void createModel()
{
    for (int i = 0; i < dataset.size(); i++)
        btreemap.insert(dataset[i]);

    // // lognormalDistribution params
    // params firstStageParams(0.001, 100000, 8, 0.0001, 0.666);
    // params secondStageParams(0.001, 100000, 8, 0.0001, 0.666);

    // uniformDistrubution params
    params firstStageParams(0.00001, 5000, 8, 0.0001, 0.00001);
    params secondStageParams(0.0000001, 1, 10000, 8, 0.0, 0.0);

    // SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 2000, 15, 800);
    SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 2000, 15, 800);
    SRMI_normal.train();
    cout << "SRMI_normal init over!" << endl;
    cout << "****************" << endl;

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
        obj.find(dataset[i].first);
    }
    QueryPerformanceCounter(&e);
    time0 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Find time:" << time0 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        obj.insert(insertDataset[i]);
    QueryPerformanceCounter(&e);
    time1 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Insert time:" << time1 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        obj.update({insertDataset[i].first, 1.11});
    QueryPerformanceCounter(&e);
    time2 += (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart;
    cout << "Update time:" << time2 << endl;

    QueryPerformanceCounter(&s);
    for (int i = 0; i < insertDataset.size(); i++)
        obj.del(insertDataset[i].first);
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
    uniformDataset uniData = uniformDataset(datasetSize, 0.9);
    uniData.generateDataset(dataset, insertDataset);

    // // generateDataset: lognormal dataset
    // lognormalDataset logData = lognormalDataset(datasetSize, 0.9);
    // logData.generateDataset(dataset, insertDataset);
    

    createModel();
    double btree_time0 = 0.0, btree_time1 = 0.0, btree_time2 = 0.0, btree_time3 = 0.0;
    double SCALE_gapped_time0 = 0.0, SCALE_gapped_time1 = 0.0, SCALE_gapped_time2 = 0.0, SCALE_gapped_time3 = 0.0;
    double ARMI_gapped_time0 = 0.0, ARMI_gapped_time1 = 0.0, ARMI_gapped_time2 = 0.0, ARMI_gapped_time3 = 0.0;
    double SRMI_gapped_time0 = 0.0, SRMI_gapped_time1 = 0.0, SRMI_gapped_time2 = 0.0, SRMI_gapped_time3 = 0.0;
    double SCALE_normal_time0 = 0.0, SCALE_normal_time1 = 0.0, SCALE_normal_time2 = 0.0, SCALE_normal_time3 = 0.0;
    double ARMI_normal_time0 = 0.0, ARMI_normal_time1 = 0.0, ARMI_normal_time2 = 0.0, ARMI_normal_time3 = 0.0;
    double SRMI_normal_time0 = 0.0, SRMI_normal_time1 = 0.0, SRMI_normal_time2 = 0.0, SRMI_normal_time3 = 0.0;
    int repetitions = 1;
    for (int i = 0; i < repetitions; i++)
    {
        cout << "btree:    " << i << endl;
        btree_test(btree_time0, btree_time1, btree_time2, btree_time3);
        cout << endl;
        printResult((i + 1), btree_time0, btree_time1, btree_time2, btree_time3);
        cout << "-------------------------------" << endl;

        cout << "SCALE_gapped:    " << i << endl;
        test(SCALE_gapped, SCALE_gapped_time0, SCALE_gapped_time1, SCALE_gapped_time2, SCALE_gapped_time3);
        cout << endl;
        printResult((i + 1), SCALE_gapped_time0, SCALE_gapped_time1, SCALE_gapped_time2, SCALE_gapped_time3);
        cout << "-------------------------------" << endl;

        cout << "ARMI_gapped:    " << i << endl;
        test(ARMI_gapped, ARMI_gapped_time0, ARMI_gapped_time1, ARMI_gapped_time2, ARMI_gapped_time3);
        cout << endl;
        printResult((i + 1), ARMI_gapped_time0, ARMI_gapped_time1, ARMI_gapped_time2, ARMI_gapped_time3);
        cout << "-------------------------------" << endl;

        cout << "SRMI_gapped:    " << i << endl;
        test(SRMI_gapped, SRMI_gapped_time0, SRMI_gapped_time1, SRMI_gapped_time2, SRMI_gapped_time3);
        cout << endl;
        printResult((i + 1), SRMI_gapped_time0, SRMI_gapped_time1, SRMI_gapped_time2, SRMI_gapped_time3);
        cout << "-------------------------------" << endl;

        cout << "SCALE_normal:    " << i << endl;
        test(SCALE_normal, SCALE_normal_time0, SCALE_normal_time1, SCALE_normal_time2, SCALE_normal_time3);
        cout << endl;
        printResult((i + 1), SCALE_normal_time0, SCALE_normal_time1, SCALE_normal_time2, SCALE_normal_time3);
        cout << "-------------------------------" << endl;

        cout << "ARMI_normal:    " << i << endl;
        test(ARMI_normal, ARMI_normal_time0, ARMI_normal_time1, ARMI_normal_time2, ARMI_normal_time3);
        cout << endl;
        printResult((i + 1), ARMI_normal_time0, ARMI_normal_time1, ARMI_normal_time2, ARMI_normal_time3);
        cout << "-------------------------------" << endl;

        cout << "SRMI_normal:    " << i << endl;
        test(SRMI_normal, SRMI_normal_time0, SRMI_normal_time1, SRMI_normal_time2, SRMI_normal_time3);
        cout << endl;
        printResult((i + 1), SRMI_normal_time0, SRMI_normal_time1, SRMI_normal_time2, SRMI_normal_time3);
        cout << "-------------------------------" << endl;

        createModel();
    }

    cout << "btreemap:" << endl;
    printResult(repetitions, btree_time0, btree_time1, btree_time2, btree_time3);

    cout << "SCALE_gapped:" << endl;
    printResult(repetitions, SCALE_gapped_time0, SCALE_gapped_time1, SCALE_gapped_time2, SCALE_gapped_time3);

    cout << "ARMI_gapped:" << endl;
    printResult(repetitions, ARMI_gapped_time0, ARMI_gapped_time1, ARMI_gapped_time2, ARMI_gapped_time3);

    cout << "SRMI_gapped:" << endl;
    printResult(repetitions, SRMI_gapped_time0, SRMI_gapped_time1, SRMI_gapped_time2, SRMI_gapped_time3);

    cout << "SCALE_normal:" << endl;
    printResult(repetitions, SCALE_normal_time0, SCALE_normal_time1, SCALE_normal_time2, SCALE_normal_time3);

    cout << "ARMI_normal:" << endl;
    printResult(repetitions, ARMI_normal_time0, ARMI_normal_time1, ARMI_normal_time2, ARMI_normal_time3);

    cout << "SRMI_normal:" << endl;
    printResult(repetitions, SRMI_normal_time0, SRMI_normal_time1, SRMI_normal_time2, SRMI_normal_time3);

    vector<pair<double, double>> totalData;
    for (int i = 0; i < dataset.size(); i++)
        totalData.push_back(dataset[i]);
    for (int i = 0; i < insertDataset.size(); i++)
        totalData.push_back(insertDataset[i]);
    std::sort(totalData.begin(), totalData.end());
    vector<pair<int, int>> cnt;
    for (int i = 0; i < datasetSize; i++)
    {
        if ((i + 1) % 10 == 0)
            cnt.push_back({0, 1});
        else
            cnt.push_back({1, 0});
    }
    reconstruction(totalData, cnt);

    return 0;
}