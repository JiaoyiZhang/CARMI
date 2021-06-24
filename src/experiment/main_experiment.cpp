/**
 * @file main_experiment.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <algorithm>
#include <vector>

// #include "../baseline/art_tree_baseline.h"
#include "../baseline/btree_baseline.h"
#include "../baseline/workloads.h"
#include "./experiment_params.h"
#include "./functions.h"
extern std::ofstream outRes;

/**
 * @brief test all datasets and workloads
 *
 * @param thre the kAlgorithmThreshold
 */
void mainExperiment() {
  // for range scan
  std::vector<int> length;

  // read-only
  mainSynthetic(kReadOnly, length);
  mainYCSB(kReadOnly, length);
  mainMap(kReadOnly, length);

  // write-heavy
  mainSynthetic(kWriteHeavy, length);
  mainYCSB(kWriteHeavy, length);
  mainMap(kWriteHeavy, length);

  // read-heavy
  mainSynthetic(kReadHeavy, length);
  mainYCSB(kReadHeavy, length);
  mainMap(kReadHeavy, length);

  // write-partial
  mainSynthetic(kWritePartial, length);
  mainYCSB(kWritePartial, length);
  mainMap(kWritePartial, length);

  // range scan
  srand(time(0));
  for (int i = 0; i < kDatasetSize; i++) {
    length.push_back(std::min(i + rand() % 100 + 1, kDatasetSize) - i);
  }
  mainSynthetic(kRangeScan, length);
  mainYCSB(kRangeScan, length);
  mainMap(kRangeScan, length);
}

/**
 * @brief test the synthetic datasets
 *
 * @param initRatio the workload type
 * @param thre the kAlgorithmThreshold
 * @param length the length of range scan
 */
void mainSynthetic(double initRatio, const std::vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  double init = initRatio;
  if (init == kRangeScan) {
    init = kReadHeavy;
  }
  LognormalDataset logData = LognormalDataset(init);
  UniformDataset uniData = UniformDataset(init);
  NormalDataset norData = NormalDataset(init);
  ExponentialDataset expData = ExponentialDataset(init);

  DataVecType initData;
  DataVecType testInsert;
  std::vector<int> trainInsertIndex;

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++"
              << std::endl;
    uniData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_btree(false, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_alex(false, initRatio, initData, testInsert, length);
      // test_radix_spline(true, initRatio, initData, testInsert, length);
      // test_radix_spline(false, initRatio, initData, testInsert, length);
    }

    CoreCARMI(false, initRatio, kRate, length, initData, testInsert);
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
              << std::endl;
    expData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_btree(false, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_alex(false, initRatio, initData, testInsert, length);
    }
    CoreCARMI(false, initRatio, kRate, length, initData, testInsert);
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
              << std::endl;
    norData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_btree(false, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_alex(false, initRatio, initData, testInsert, length);
      // test_radix_spline(true, initRatio, initData, testInsert, length);
      // test_radix_spline(false, initRatio, initData, testInsert, length);
    }
    CoreCARMI(false, initRatio, kRate, length, initData, testInsert);
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    std::cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++"
              << std::endl;
    logData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_btree(false, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_alex(false, initRatio, initData, testInsert, length);
      // test_radix_spline(true, initRatio, initData, testInsert, length);
      // test_radix_spline(false, initRatio, initData, testInsert, length);
    }
    CoreCARMI(false, initRatio, kRate, length, initData, testInsert);
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    outRes << std::endl;
  }
}

/**
 * @brief test the map datasets
 *
 * @param initRatio the workload type
 * @param thre the kAlgorithmThreshold
 * @param length the length of range scan
 */
void mainMap(double initRatio, const std::vector<int> &length) {
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  std::cout << "construct map" << std::endl;
  outRes << "construct map" << std::endl;
  double init = initRatio;
  if (init == kRangeScan) {
    init = kReadHeavy;
  }
  LongitudesDataset longData = LongitudesDataset(init);
  LonglatDataset latData = LonglatDataset(init);

  DataVecType initData;
  DataVecType trainFind;
  DataVecType trainInsert;
  DataVecType testInsert;
  std::vector<int> trainInsertIndex;

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;

    std::cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++"
              << std::endl;
    latData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_radix_spline(true, initRatio, initData, testInsert, length);
    }
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
              << std::endl;
    longData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      // test_btree(true, initRatio, initData, testInsert, length);
      // test_alex(true, initRatio, initData, testInsert, length);
      // test_radix_spline(true, initRatio, initData, testInsert, length);
    }
    CoreCARMI(true, initRatio, kRate, length, initData, testInsert);

    outRes << std::endl;
  }
}

/**
 * @brief test the YCSB datasets
 *
 * @param initRatio the workload type
 * @param thre the kAlgorithmThreshold
 * @param length the length of range scan
 */
void mainYCSB(double initRatio, const std::vector<int> &length) {
  kPrimaryIndex = true;
  std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
               "&&&&&&&"
            << std::endl;
  std::cout << "initRatio is: " << initRatio << std::endl;
  outRes << "initRatio," << initRatio << std::endl;
  std::cout << "construct ycsb" << std::endl;
  outRes << "construct ycsb" << std::endl;
  double init = initRatio;
  if (init == kRangeScan) {
    init = kReadHeavy;
  }
  YCSBDataset ycsbData = YCSBDataset(init);

  DataVecType initData;
  DataVecType trainFind;
  DataVecType trainInsert;
  DataVecType testInsert;
  std::vector<int> trainInsertIndex;

  for (int r = 0; r < rate.size(); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++"
              << std::endl;
    ycsbData.GenerateDataset(&initData, &testInsert);
    if (r == 0) {
      test_btree(true, initRatio, initData, testInsert, length);
      test_alex(true, initRatio, initData, testInsert, length);
      test_radix_spline(true, initRatio, initData, testInsert, length);
    }
    CoreExternalCARMI(true, initRatio, kRate, length, initData, testInsert);

    outRes << std::endl;
  }
  kPrimaryIndex = false;
}
