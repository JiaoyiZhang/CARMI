/**
 * @file main_experiment.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_MAIN_EXPERIMENT_H_
#define SRC_EXPERIMENT_MAIN_EXPERIMENT_H_

#include <vector>

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"
#include "./core.h"
#include "dataset/exponential_distribution.h"
#include "dataset/lognormal_distribution.h"
#include "dataset/longitudes.h"
#include "dataset/longlat.h"
#include "dataset/normal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/ycsb.h"
extern ofstream outRes;

void mainSynthetic(double initRatio, int thre, const vector<int> &length);
void mainYCSB(double initRatio, int thre, const vector<int> &length);
void mainMap(double initRatio, int thre, const vector<int> &length);

void mainExperiment(int thre) {
  // for range scan
  vector<int> length;

  // read-only
  mainSynthetic(kReadOnly, thre, length);
  mainYCSB(kReadOnly, thre, length);
  mainMap(kReadOnly, thre, length);

  // write-heavy
  mainSynthetic(kWriteHeavy, thre, length);
  mainYCSB(kWriteHeavy, thre, length);
  mainMap(kWriteHeavy, thre, length);

  // read-heavy
  mainSynthetic(kReadHeavy, thre, length);
  mainYCSB(kReadHeavy, thre, length);
  mainMap(kReadHeavy, thre, length);

  // write-partial
  mainSynthetic(kWritePartial, thre, length);
  mainYCSB(kWritePartial, thre, length);
  mainMap(kWritePartial, thre, length);

  // range scan
  srand(time(0));
  for (int i = 0; i < kDatasetSize; i++) {
    length.push_back(min(i + rand() % 100 + 1, kDatasetSize) - i);
  }
  mainSynthetic(kRangeScan, thre, length);
  mainYCSB(kRangeScan, thre, length);
  mainMap(kRangeScan, thre, length);
}

void mainSynthetic(double initRatio, int thre, const vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << endl;
  std::cout << "initRatio is: " << initRatio << endl;
  outRes << "initRatio," << initRatio << endl;
  LognormalDataset logData = LognormalDataset(initRatio);
  UniformDataset uniData = UniformDataset(initRatio);
  NormalDataset norData = NormalDataset(initRatio);
  ExponentialDataset expData = ExponentialDataset(initRatio);

  DataVectorType initData;
  DataVectorType trainFind;
  DataVectorType trainInsert;
  DataVectorType testInsert;

  vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5

  // for (int r = 0; r < rate.size(); r++)
  for (int r = 3; r < 4; r++) {
    double kRate;
    if (initRatio == 0.5)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << endl;
    cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++" << endl;
    uniData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
              << endl;
    expData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
              << endl;
    norData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    std::cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++"
              << endl;
    logData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    outRes << endl;
  }
}

void mainMap(double initRatio, int thre, const vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << endl;
  std::cout << "initRatio is: " << initRatio << endl;
  outRes << "initRatio," << initRatio << endl;
  std::cout << "construct map" << endl;
  outRes << "construct map" << endl;
  std::cout << "kAlgThre:" << thre << endl;
  LongitudesDataset longData = LongitudesDataset(initRatio);
  LonglatDataset latData = LonglatDataset(initRatio);

  DataVectorType initData;
  DataVectorType trainFind;
  DataVectorType trainInsert;
  DataVectorType testInsert;

  vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5

  // for (int r = 0; r < rate.size(); r++)
  for (int r = 3; r < 4; r++) {
    double kRate;
    if (initRatio == 0.5)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << endl;
    std::cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++"
              << endl;
    latData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
              << endl;
    longData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    outRes << endl;
  }
}

void mainYCSB(double initRatio, int thre, const vector<int> &length) {
  kPrimaryIndex = true;
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << endl;
  std::cout << "initRatio is: " << initRatio << endl;
  outRes << "initRatio," << initRatio << endl;
  std::cout << "construct ycsb" << endl;
  outRes << "construct ycsb" << endl;
  double init = initRatio;
  if (init == 2) init = 0.95;
  YCSBDataset ycsbData = YCSBDataset(init);

  DataVectorType initData;
  DataVectorType trainFind;
  DataVectorType trainInsert;
  DataVectorType testInsert;

  vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5
  // for (int r = 0; r < rate.size(); r++)
  for (int r = 4; r < 5; r++) {
    double kRate;
    if (initRatio == 0.5)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << endl;
    std::cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++" << endl;
    ycsbData.GenerateDataset(&initData, &trainFind, &trainInsert, &testInsert);
    if (r == 3) {
      // btree_test(initRatio, initData, testInsert, length);
      // artTree_test(initRatio, initData, testInsert, length);
    }
    Core(initRatio, kRate, thre, length, &initData, &trainFind, &trainInsert,
         &testInsert);

    outRes << endl;
  }
  kPrimaryIndex = false;
}

#endif  // SRC_EXPERIMENT_MAIN_EXPERIMENT_H_
