#include <iostream>
#include "learnedIndex.h"
#include "segmentationModel.h"
#include "gappedArray.h"
#include <algorithm>
#include <random>
#include <iomanip>
const int datasetSize = 10000;
vector<pair<double, double>> dataset;

int main()
{
    float maxValue = 10000.00;
    btree::btree_map<double, double> btreemap;

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
        // if (i % 10 != 0)
        // {
        dataset.push_back({double(ds[i] * factor), double(ds[i] * factor) * 10});
        btreemap.insert({double(ds[i] * factor), double(ds[i] * factor) * 10});
        // }
    }

    params firstStageParams;
    firstStageParams.learningRate = 0.01;
    firstStageParams.maxEpoch = 1000;
    firstStageParams.neuronNumber = 8;

    params secondStageParams;
    secondStageParams.learningRate = 0.01;
    secondStageParams.maxEpoch = 1000;
    secondStageParams.neuronNumber = 8;

    totalModel rmi(dataset, firstStageParams, secondStageParams, 1024, 128, 1000);
    rmi.sortData();
    rmi.train();

    segModel seg(dataset);
    seg.sortData();
    seg.preProcess();

    gappedArray ga(dataset, 200);


    clock_t RMI_start, RMI_end, BTree_start, BTree_end, seg_start, seg_end, ga_start, ga_end;
    int repetitions = 10000;
    RMI_start = clock();
    for (int k = 0; k < repetitions; k++)
    {
        for (int i = 0; i < datasetSize; i++)
        {
            rmi.find(dataset[i].first);
        }
    }
    RMI_end = clock();
    double rmi_time = static_cast<double>(RMI_end - RMI_start) / CLOCKS_PER_SEC;

    seg_start = clock();
    for (int k = 0; k < repetitions; k++)
    {
        for (int i = 0; i < datasetSize; i++)
        {
            seg.find(dataset[i].first);
        }
    }
    seg_end = clock();
    double seg_time = static_cast<double>(seg_end - seg_start) / CLOCKS_PER_SEC;

    
    ga_start = clock();
    for (int k = 0; k < repetitions; k++)
    {
        for (int i = 0; i < datasetSize; i++)
        {
            ga.find(dataset[i].first);
        }
    }
    ga_end = clock();
    double ga_time = static_cast<double>(ga_end - ga_start) / CLOCKS_PER_SEC;

    BTree_start = clock();
    for (int k = 0; k < repetitions; k++)
    {
        for (int i = 0; i < datasetSize; i++)
        {
            btreemap.find(dataset[i].first);
        }
    }
    BTree_end = clock();
    double btree_time = static_cast<double>(BTree_end - BTree_start) / CLOCKS_PER_SEC;

    std::cout << "rmi time: " << double(rmi_time / double(datasetSize)) << std::endl;
    std::cout << "seg time: " << double(seg_time / double(datasetSize)) << std::endl;
    std::cout << "ga time: " << double(ga_time / double(datasetSize)) << std::endl;
    std::cout << "btree time: " << double(btree_time / double(datasetSize)) << std::endl;

    // for (int i = 0; i < ds.size(); i++)
    // {
    //     if (i % 10 == 0)
    //     {
    //         rmi.insert(double(ds[i] * factor), double(ds[i] * factor) * 10);
    //         btreemap.insert({double(ds[i] * factor), double(ds[i] * factor) * 10});
    //     }
    // }

    // RMI_start = clock();
    // for (int k = 0; k < repetitions; k++)
    // {
    //     for (int i = 0; i < ds.size(); i += 10)
    //     {
    //         rmi.find(double(ds[i] * factor));
    //     }
    // }
    // RMI_end = clock();
    // rmi_time = static_cast<double>(RMI_end - RMI_start) / CLOCKS_PER_SEC;

    // BTree_start = clock();
    // for (int k = 0; k < repetitions; k++)
    // {
    //     for (int i = 0; i < ds.size(); i += 10)
    //     {
    //         btreemap.find(double(ds[i] * factor));
    //     }
    // }
    // BTree_end = clock();
    // btree_time = static_cast<double>(BTree_end - BTree_start) / CLOCKS_PER_SEC;

    // std::cout << "rmi time (after insert) : " << double(rmi_time / double(repetitions)) << std::endl;
    // std::cout << "btree time (after insert) : " << double(btree_time / double(repetitions)) << std::endl;

    return 0;
}
