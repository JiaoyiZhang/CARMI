#ifndef RECONSTRUCTION_H
#define RECONSTRUCTION_H

#include "./trainModel/lr.h"
#include "./trainModel/nn.h"
#include "./innerNode/adaptiveRMI.h"
#include "./innerNode/staticRMI.h"
#include "./innerNode/scaleModel.h"
#include "./leafNode/gappedNode.h"
#include "./leafNode/normalNode.h"
#include "./dataset/lognormalDistribution.h"
#include "./dataset/uniformDistribution.h"

#include <algorithm>
#include <iostream>
#include <windows.h>
#include <vector>
using namespace std;

void reconstruction(const vector<pair<double, double>> &data, const vector<pair<int, int>> &cnt)
{
    cout << "-------------------------------" << endl;
    cout << "Start reconstruction!" << endl;
    int datasetSize = data.size();
    vector<pair<double, double>> dataset = data;
    int totalReadNum = 0, totalWriteNum = 0;
    for (int i = 0; i < datasetSize; i++)
    {
        totalReadNum += cnt[i].first;
        totalWriteNum += cnt[i].second;
    }

    // // lognormalDistribution params
    // params firstStageParams(0.001, 100000, 8, 0.0001, 0.666);
    // params secondStageParams(0.001, 100000, 8, 0.0001, 0.666);

    // uniformDistrubution params
    params firstStageParams(0.00001, 5000, 8, 0.0001, 0.00001);
    params secondStageParams(0.0000001, 1, 10000, 8, 0.0, 0.0);

    if ((float)totalReadNum / (float)totalWriteNum >= 4.0)
    {
        // use static inner node
        staticRMI<normalNode<linearRegression>, linearRegression> SRMI_normal;

        // SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 2000, 15, 800);
        SRMI_normal = staticRMI<normalNode<linearRegression>, linearRegression>(dataset, firstStageParams, secondStageParams, 2000, 15, 800);
        SRMI_normal.train();
        cout << "SRMI_normal init over!" << endl;
        cout << "****************" << endl;
        SRMI_normal.change(cnt, 2000, secondStageParams, 800);

        //test
        LARGE_INTEGER s, e, c;
        QueryPerformanceCounter(&s);
        QueryPerformanceFrequency(&c);
        for (int i = 0; i < dataset.size(); i++)
        {
            SRMI_normal.find(dataset[i].first);
        }
        QueryPerformanceCounter(&e);
        cout << "Find time:" << (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart / (float)dataset.size() << endl;
    }
    else
    {
        scaleModel<normalNode<linearRegression>> SCALE_normal;
        // SCALE_normal = scaleModel<normalNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_normal = scaleModel<normalNode<linearRegression>>(secondStageParams, 1000, 100, 800);
        SCALE_normal.initialize(dataset);
        cout << "SCALE_normal init over!" << endl;
        cout << "****************" << endl;
        SCALE_normal.change(cnt, 1000, secondStageParams, 800);

        //test
        LARGE_INTEGER s, e, c;
        QueryPerformanceCounter(&s);
        QueryPerformanceFrequency(&c);
        for (int i = 0; i < dataset.size(); i++)
        {
            SCALE_normal.find(dataset[i].first);
        }
        QueryPerformanceCounter(&e);
        cout << "Find time:" << (double)(e.QuadPart - s.QuadPart) / (double)c.QuadPart / (float)dataset.size() << endl;
    }
}

#endif