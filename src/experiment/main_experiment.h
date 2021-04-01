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

#include <algorithm>
#include <vector>

#include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"
#include "../params.h"
#include "./core.h"
#include "dataset/exponential_distribution.h"
#include "dataset/lognormal_distribution.h"
#include "dataset/longitudes.h"
#include "dataset/longlat.h"
#include "dataset/normal_distribution.h"
#include "dataset/uniform_distribution.h"
#include "dataset/ycsb.h"
extern std::ofstream outRes;

void mainSynthetic(double initRatio, int thre, const std::vector<int> &length);
void mainYCSB(double initRatio, int thre, const std::vector<int> &length);
void mainMap(double initRatio, int thre, const std::vector<int> &length);

void mainExperiment(int thre) {
  // for range scan
  std::vector<int> length;

  // read-only
  mainSynthetic(carmi_params::kReadOnly, thre, length);
  mainYCSB(carmi_params::kReadOnly, thre, length);
  mainMap(carmi_params::kReadOnly, thre, length);

  // write-heavy
  mainSynthetic(carmi_params::kWriteHeavy, thre, length);
  mainYCSB(carmi_params::kWriteHeavy, thre, length);
  mainMap(carmi_params::kWriteHeavy, thre, length);

  // read-heavy
  mainSynthetic(carmi_params::kReadHeavy, thre, length);
  mainYCSB(carmi_params::kReadHeavy, thre, length);
  mainMap(carmi_params::kReadHeavy, thre, length);

  // write-partial
  mainSynthetic(carmi_params::kWritePartial, thre, length);
  mainYCSB(carmi_params::kWritePartial, thre, length);
  mainMap(carmi_params::kWritePartial, thre, length);

  // range scan
  srand(time(0));
  for (int i = 0; i < carmi_params::kDatasetSize; i++) {
    length.push_back(
        std::min(i + rand() % 100 + 1, carmi_params::kDatasetSize) - i);
  }
  mainSynthetic(carmi_params::kRangeScan, thre, length);
  mainYCSB(carmi_params::kRangeScan, thre, length);
  mainMap(carmi_params::kRangeScan, thre, length);
}

void mainSynthetic(double initRatio, int thre, const std::vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  double init = initRatio;
  if (init == carmi_params::kRangeScan) {
    init = carmi_params::kReadHeavy;
  }
  LognormalDataset logData = LognormalDataset(init);
  UniformDataset uniData = UniformDataset(init);
  NormalDataset norData = NormalDataset(init);
  ExponentialDataset expData = ExponentialDataset(init);

  carmi_params::TestDataVecType initData;
  carmi_params::TestDataVecType trainFind;
  carmi_params::TestDataVecType trainInsert;
  carmi_params::TestDataVecType testInsert;
  std::vector<int> trainInsertIndex;

#ifdef DEBUG
  std::vector<double> rate = {0.2};
  std::vector<double> rate1 = {0.1};  // 0.5
#endif                                // DEBUG
#ifndef DEBUG
  std::vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  std::vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5
#endif                                                              // !DEBUG

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == carmi_params::kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++"
              << std::endl;
    uniData.GenerateDataset(&initData, &trainFind, &trainInsert,
                            &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
              << std::endl;
    expData.GenerateDataset(&initData, &trainFind, &trainInsert,
                            &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
              << std::endl;
    norData.GenerateDataset(&initData, &trainFind, &trainInsert,
                            &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    std::cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++"
              << std::endl;
    logData.GenerateDataset(&initData, &trainFind, &trainInsert,
                            &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    outRes << std::endl;
  }
}

void mainMap(double initRatio, int thre, const std::vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  std::cout << "construct map" << std::endl;
  outRes << "construct map" << std::endl;
  std::cout << "kAlgThre:" << thre << std::endl;
  double init = initRatio;
  if (init == carmi_params::kRangeScan) {
    init = carmi_params::kReadHeavy;
  }
  LongitudesDataset longData = LongitudesDataset(init);
  LonglatDataset latData = LonglatDataset(init);

  carmi_params::TestDataVecType initData;
  carmi_params::TestDataVecType trainFind;
  carmi_params::TestDataVecType trainInsert;
  carmi_params::TestDataVecType testInsert;
  std::vector<int> trainInsertIndex;

#ifdef DEBUG
  std::vector<double> rate = {0.2};
  std::vector<double> rate1 = {0.1};  // 0.5
#endif                                // DEBUG
#ifndef DEBUG
  std::vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  std::vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5
#endif                                                              // !DEBUG

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == carmi_params::kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;

    std::cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++"
              << std::endl;
    latData.GenerateDataset(&initData, &trainFind, &trainInsert,
                            &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
              << std::endl;
    longData.GenerateDataset(&initData, &trainFind, &trainInsert,
                             &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    outRes << std::endl;
  }
}

void mainYCSB(double initRatio, int thre, const std::vector<int> &length) {
  carmi_params::kPrimaryIndex = true;
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  std::cout << "construct ycsb" << std::endl;
  outRes << "construct ycsb" << std::endl;
  double init = initRatio;
  if (init == carmi_params::kRangeScan) {
    init = carmi_params::kReadHeavy;
  }
  YCSBDataset ycsbData = YCSBDataset(init);

  carmi_params::TestDataVecType initData;
  carmi_params::TestDataVecType trainFind;
  carmi_params::TestDataVecType trainInsert;
  carmi_params::TestDataVecType testInsert;
  std::vector<int> trainInsertIndex;

#ifdef DEBUG
  std::vector<double> rate = {0.2};
  std::vector<double> rate1 = {0.1};  // 0.5
#endif                                // DEBUG
#ifndef DEBUG
  std::vector<double> rate = {0.3, 0.25, 0.22, 0.2, 0.1};
  std::vector<double> rate1 = {0.25, 0.2, 0.15, 0.1, 0.075, 0.05};  // 0.5
#endif                                                              // !DEBUG

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == carmi_params::kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++"
              << std::endl;
    ycsbData.GenerateDataset(&initData, &trainFind, &trainInsert,
                             &trainInsertIndex, &testInsert);
#ifdef BASELINE
    btree_test(initRatio, initData, testInsert, length);
    artTree_test(initRatio, initData, testInsert, length);
#endif  // BASELINE
    Core(initRatio, kRate, thre, length, trainInsertIndex, initData, trainFind,
         trainInsert, testInsert);

    outRes << std::endl;
  }
  carmi_params::kPrimaryIndex = false;
}

#endif  // SRC_EXPERIMENT_MAIN_EXPERIMENT_H_
