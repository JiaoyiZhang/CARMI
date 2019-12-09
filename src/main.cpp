#include <iostream>
#include "rmi.h"
#include <algorithm>
#include <random>
const int datasetSize = 1000000;
std::array<int, datasetSize> dataset;

int main()
{
    float maxValue = 1000000.00;

    // create dataset randomly
    std::default_random_engine generator;
    std::lognormal_distribution<double> distribution(0.0, 2.0);
    std::array<double, datasetSize> ds;

    for (int i = 0; i < datasetSize; i++)
    {
        ds[i] = distribution(generator);
    }
    std::sort(ds.begin(), ds.end());

    double maxV = ds[ds.size() - 1];
    double factor = maxValue / maxV;
    for (int i = 0; i < ds.size(); i++)
    {
        dataset[i] = static_cast<int>(ds[i] * factor);
    }

    params firstStageParams;
    firstStageParams.batchSize = 4096;
    firstStageParams.learningRate = 0.01;
    firstStageParams.maxNumEpochs = 50000;
    firstStageParams.neuronNumber = 8;

    params secondStageParams;
    secondStageParams.batchSize = 1024;
    secondStageParams.learningRate = 0.01;
    secondStageParams.maxNumEpochs = 5000;

    RMI<int, int, 128> recursiveModelIndex(firstStageParams, secondStageParams, 20480);
    btree::btree_map<int, int> btreemap;

    for (int i = 0; i < datasetSize; i++)
    {
        recursiveModelIndex.insert(dataset[i], dataset[i + 1]);
        btreemap.insert({dataset[i], dataset[i + 1]});
    }
    recursiveModelIndex.sortData();

    recursiveModelIndex.train();

    std::vector<double> rmiDurations;
    std::vector<double> btreeDurations;
    clock_t RMI_start, RMI_end, BTree_start, BTree_end;
    RMI_start = clock();
    for (int i = 0; i < datasetSize; i++)
    {
        recursiveModelIndex.find(dataset[i]);
    }
    RMI_end = clock();
    double rmi_time = (double)(RMI_end - RMI_start) / CLOCKS_PER_SEC;

    BTree_start = clock();
    for (int i = 0; i < datasetSize; i++)
    {
        btreemap.find(dataset[i]);
    }
    BTree_end = clock();
    double btree_time = (double)(BTree_end - BTree_start) / CLOCKS_PER_SEC;

    std::cout << "rmi time: " << rmi_time / datasetSize << std::endl;
    std::cout << "btree time: " << btree_time / datasetSize << std::endl;

    return 0;
}
