#include "./trainModel/lr.h"
#include "./trainModel/nn.h"
#include "./innerNode/adaptiveRMI.h"
#include "./innerNode/staticRMI.h"
#include "./innerNode/scaleModel.h"
#include "./leafNode/gappedNode.h"
#include "./leafNode/normalNode.h"

#include <algorithm>
#include <random>
#include <iostream>

int datasetSize = 10000;
vector<pair<double, double>> dataset;
vector<pair<double, double>> insertDataset;

btree::btree_map<double, double> btreemap;

staticRMI<normalNode<linearRegression>, linearRegression> SRMI_normal;
staticRMI<gappedNode<linearRegression>, linearRegression> SRMI_gapped;
adaptiveRMI<gappedNode<linearRegression>, linearRegression> ARMI_gapped;
scaleModel<gappedNode<linearRegression>> SCALE_gapped;

void generateDataset()
{
    float maxValue = 10000.00;

    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);
    vector<double> ds;

    for (int i = 0; i < datasetSize; i++)
    {
        ds.push_back(distribution(generator));
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
    for (int i = 0; i < ds.size(); i++)
    {
        if (i % 10 != 0)
        {
            dataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
            btreemap.insert({double(ds[i] * factor), double(ds[i] * factor) * 10});
        }
        else
            insertDataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
    }
    datasetSize = dataset.size();
}

void createModel()
{
    params firstStageParams(0.001, 100000, 8);
    params secondStageParams(0.001, 100000, 8);

    SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 1024, 128, 200);
    SRMI_normal.train();
    cout << "SRMI_normal init over!" << endl;

    SRMI_gapped = staticRMI<gappedNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 5000, 128, 800);
    SRMI_gapped.train();
    cout << "SRMI_gapped init over!" << endl;

    ARMI_gapped = adaptiveRMI<gappedNode<linearRegression>, linearRegression>(firstStageParams, secondStageParams, 1000, 12, 800);
    ARMI_gapped.initialize(dataset);
    cout << "ARMI_gapped init over!" << endl;

    SCALE_gapped = scaleModel<gappedNode<linearRegression>>(secondStageParams, 1000, 100, 800);
    SCALE_gapped.initialize(dataset);
    cout << "SCALE_gapped init over!" << endl;
}

template <typename type>
void find(type obj)
{
    for (int i = 0; i < dataset.size(); i++)
    {
        obj.find(dataset[i].first);
    }
}
template <typename type>
void insert(type obj)
{
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.insert(insertDataset[i]);
    }
}
template <typename type>
void update(type obj)
{
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.update({insertDataset[i].first, 1.11});
    }
}
template <typename type>
void del(type obj)
{
    for (int i = 0; i < insertDataset.size(); i++)
    {
        obj.del(insertDataset[i].first);
    }
}
template <typename type>
void test(type obj, double &time0, double &time1, double &time2, double &time3)
{
    clock_t s, e;
    s = clock();
    find(obj);
    e = clock();
    time0 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Find time:" << static_cast<double>(e - s) / CLOCKS_PER_SEC / float(datasetSize) << endl;

    s = clock();
    insert(obj);
    e = clock();
    time1 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Insert time:" << static_cast<double>(e - s) / CLOCKS_PER_SEC / float(insertDataset.size()) << endl;

    s = clock();
    update(obj);
    e = clock();
    time2 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Update time:" << static_cast<double>(e - s) / CLOCKS_PER_SEC / float(insertDataset.size()) << endl;

    s = clock();
    del(obj);
    e = clock();
    time3 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Delete time:" << static_cast<double>(e - s) / CLOCKS_PER_SEC / float(insertDataset.size()) << endl;
}
void btreetest(double &time0, double &time1, double &time2, double &time3)
{
    clock_t s, e;
    s = clock();
    for (int i = 0; i < dataset.size(); i++)
    {
        btreemap.find(dataset[i].first);
    }
    e = clock();
    time0 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Find time:" << time0 / (float)datasetSize << endl;

    s = clock();
    for (int i = 0; i < insertDataset.size(); i++)
    {
        btreemap.insert(insertDataset[i]);
    }
    e = clock();
    time1 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Insert time:" << time1 / float(insertDataset.size()) << endl;

    s = clock();
    for (int i = 0; i < insertDataset.size(); i++)
    {
        btreemap.find(insertDataset[i].first);
    }
    e = clock();
    time2 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Update time:" << time2 / float(insertDataset.size()) << endl;

    s = clock();
    for (int i = 0; i < insertDataset.size(); i++)
    {
        btreemap.erase(insertDataset[i].first);
    }
    e = clock();
    time3 += static_cast<double>(e - s) / CLOCKS_PER_SEC;
    cout << "Delete time:" << time3 / float(insertDataset.size()) << endl;
}
void printResult(int r, double &time0, double &time1, double &time2, double &time3)
{
    cout << "Average time: " << endl;
    cout << "Find time:" << time0 / float(datasetSize) / float(r) << endl;
    cout << "Insert time:" << time1 / float(insertDataset.size()) / float(r) << endl;
    cout << "Update time:" << time2 / float(insertDataset.size()) / float(r) << endl;
    cout << "Delete time:" << time3 / float(insertDataset.size()) / float(r) << endl;
    cout << "***********************" << endl;
    time0 = 0;
    time1 = 0;
    time2 = 0;
    time3 = 0;
}
int main()
{
    generateDataset();
    createModel();
    clock_t s, e;
    double time0 = 0.0, time1 = 0.0, time2 = 0.0, time3 = 0.0;
    int repetitions = 100;
    for (int i = 0; i < repetitions; i++)
    {
        cout << "btree:    " << i << endl;
        btreetest(time0, time1, time2, time3);
        cout << endl;
    }
    cout << "btreemap:" << endl;
    printResult(repetitions, time0, time1, time2, time3);

    for (int i = 0; i < repetitions; i++)
    {
        cout << "SRMI_normal:    " << i << endl;
        test(SRMI_normal, time0, time1, time2, time3);
        cout << endl;
    }
    cout << "SRMI_normal:" << endl;
    printResult(repetitions, time0, time1, time2, time3);

    for (int i = 0; i < repetitions; i++)
    {
        cout << "SRMI_gapped:    " << i << endl;
        test(SRMI_gapped, time0, time1, time2, time3);
        cout << endl;
    }
    cout << "SRMI_gapped:" << endl;
    printResult(repetitions, time0, time1, time2, time3);

    for (int i = 0; i < repetitions; i++)
    {
        cout << "btree:    " << i << endl;
        test(ARMI_gapped, time0, time1, time2, time3);
        cout << endl;
    }
    cout << "ARMI_gapped:" << endl;
    printResult(repetitions, time0, time1, time2, time3);

    for (int i = 0; i < repetitions; i++)
    {
        cout << "SCALE_gapped:    " << i << endl;
        test(SCALE_gapped, time0, time1, time2, time3);
        cout << endl;
    }
    cout << "SCALE_gapped:" << endl;
    printResult(repetitions, time0, time1, time2, time3);
    return 0;
}