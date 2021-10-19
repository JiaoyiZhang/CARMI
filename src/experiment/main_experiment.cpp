/**
 * @file main_experiment.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <algorithm>
#include <vector>

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
  // mainYCSB(kWriteHeavy, length);
  // mainMap(kWriteHeavy, length);

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
  LognormalDataset logData(init);
  UniformDataset uniData(init);
  NormalDataset norData(init);
  ExponentialDataset expData(init);

  DataVecType initData;
  DataVecType insertData;
  DataVecType testInsert;

  for (int r = 0; r < static_cast<int>(rate.size()); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++"
              << std::endl;
    uniData.GenerateDataset(&initData, &insertData, &testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);

    std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
              << std::endl;
    expData.GenerateDataset(&initData, &insertData, &testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);

    std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
              << std::endl;
    norData.GenerateDataset(&initData, &insertData, &testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);

    std::cout << "+++++++++++ lognormal dataset ++++++++++++++++++++++++++"
              << std::endl;
    logData.GenerateDataset(&initData, &insertData, &testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);

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
  LongitudesDataset longData(init);
  LonglatDataset latData(init);

  DataVecType initData;
  DataVecType insertData;
  DataVecType testInsert;

  for (int r = 0; r < static_cast<int>(rate.size()); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;

    std::cout << "+++++++++++ longlat dataset ++++++++++++++++++++++++++"
              << std::endl;
    latData.GenerateDataset(&initData, &insertData, &testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);

    std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
              << std::endl;
    longData.GenerateDataset(&initData, &insertData, &testInsert);
    // CoreCARMI(true, initRatio, kRate, length, initData, insertData,
    // testInsert);
    CoreCARMI(false, initRatio, kRate, length, initData, insertData,
              testInsert);

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
  YCSBDataset ycsbData(init);

  DataVecType initData;
  DataVecType insertData;
  DataVecType testInsert;

  for (int r = 0; r < static_cast<int>(rate.size()); r++) {
    double kRate;
    if (initRatio == kWriteHeavy)
      kRate = rate1[r];
    else
      kRate = rate[r];
    outRes << "kRate:" << kRate << std::endl;
    std::cout << "+++++++++++ ycsb dataset ++++++++++++++++++++++++++"
              << std::endl;
    ycsbData.GenerateDataset(&initData, &insertData, &testInsert);
    CoreExternalCARMI(true, initRatio, kRate, length, initData, testInsert);

    outRes << std::endl;
  }
  kPrimaryIndex = false;
}
