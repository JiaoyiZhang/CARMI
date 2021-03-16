/**
 * @file fixed_experiment.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_FIXED_EXPERIMENT_H_
#define SRC_EXPERIMENT_FIXED_EXPERIMENT_H_
#include <algorithm>
#include <vector>

#include "./static.h"
#include "dataset/exponential_distribution.h"
#include "dataset/lognormal_distribution.h"
#include "dataset/longitudes.h"
#include "dataset/normal_distribution.h"
#include "dataset/uniform_distribution.h"

void fixedSynthetic(double initRatio, int kLeafID, const vector<int> &lengt);

void fixedExperiment() {
  // for range scan
  vector<int> length;
  // static structure
  for (int i = 0; i < 2; i++) {
    std::cout << "kleafnode:" << i << endl;
    fixedSynthetic(kReadOnly, i, length);
    if (i == 1) fixedSynthetic(kWriteHeavy, i, length);
  }
  fixedSynthetic(kReadHeavy, 0, length);
  fixedSynthetic(kWritePartial, 0, length);

  srand(time(0));
  for (int i = 0; i < kDatasetSize; i++) {
    length.push_back(min(i + rand() % 100 + 1, kDatasetSize) - i);
  }
  fixedSynthetic(kRangeScan, 0, length);
}

void fixedSynthetic(double initRatio, int kLeafID, const vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << endl;
  std::cout << "initRatio is: " << initRatio << endl;
  outRes << "initRatio," << initRatio << endl;
  double init = initRatio;
  if (init == 2) init = 0.95;
  LongitudesDataset longData = LongitudesDataset(init);
  UniformDataset uniData = UniformDataset(init);
  NormalDataset norData = NormalDataset(init);
  ExponentialDataset expData = ExponentialDataset(init);
  DataVectorType initData;
  DataVectorType trainFind;
  DataVectorType trainInsert;
  DataVectorType testInsert;

  for (int i = 0; i < 1; i++) {
    int childNum = 131072;
    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    outRes << "+++++++++++ childNum: " << childNum << endl;
    uniData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    outRes << "+++++++++++ uniform dataset++++++++++++++++++++++++++" << endl;
    RunStatic(initRatio, kLeafID, initData, testInsert, length);

    std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
              << endl;
    outRes << "+++++++++++ childNum: " << childNum << endl;
    expData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
           << endl;
    RunStatic(initRatio, kLeafID, initData, testInsert, length);

    std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
              << endl;
    outRes << "+++++++++++ childNum: " << childNum << endl;
    norData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++" << endl;
    RunStatic(initRatio, kLeafID, initData, testInsert, length);

    std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
              << endl;
    outRes << "+++++++++++ childNum: " << childNum << endl;
    longData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    outRes << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
           << endl;
    RunStatic(initRatio, kLeafID, initData, testInsert, length);
  }
}

#endif  // SRC_EXPERIMENT_FIXED_EXPERIMENT_H_
