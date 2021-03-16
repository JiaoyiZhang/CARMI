/**
 * @file workloadd.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADD_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADD_H_
#include <chrono>
#include <vector>

#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "./zipfian.h"
extern std::ofstream outRes;

// write partial workload
// a mix of 85/15 reads and writes
void WorkloadD(const DataVectorType &initDataset,
               const DataVectorType &insertDataset, CARMI *carmi) {
  auto init = initDataset;
  auto insert = insertDataset;

  std::default_random_engine engine;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed);
  shuffle(init.begin(), init.end(), engine);

  if (!kPrimaryIndex) {
    unsigned seed1 =
        std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed1);
    shuffle(insert.begin(), insert.end(), engine);
  }

  int end = round(kTestSize * (1 - kWritePartial));
  int findCnt = 0;
  int insertCnt = 0;
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, init.size());
  int length = round(kTestSize * kWritePartial);
  int insert_length = round(kTestSize * (1 - kWritePartial));
#ifdef DEBUG
  if (length != 85000)
    std::cout << "wrong, workload d, length:" << length << std::endl;
  if (insert_length != 15000)
    std::cout << "wrong, workload d, insert_length:" << insert_length
              << std::endl;
#endif  // DEBUG
  std::vector<int> index(length, 0);
  for (int i = 0; i < length; i++) {
    int idx = zip.GenerateNextIndex();
    index[i] = idx;
  }

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 17 && findCnt < init.size(); j++) {
      carmi->Find(init[index[findCnt]].first);
      findCnt++;
    }
    for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++) {
      carmi->Insert(insert[insertCnt]);
      insertCnt++;
    }
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 17 && findCnt < length; j++) {
      carmi->Find(init[findCnt].first);
      findCnt++;
    }
    for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
      carmi->Insert(insert[insertCnt]);
      insertCnt++;
    }
  }
#endif
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  insertCnt = 0;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 17 && findCnt < init.size(); j++) {
      findCnt++;
    }
    for (int j = 0; j < 3 && insertCnt < insertDataset.size(); j++) {
      insertCnt++;
    }
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 17 && findCnt < length; j++) {
      findCnt++;
    }
    for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
      insertCnt++;
    }
  }
#endif
  e = std::chrono::system_clock::now();
  double tmp0 =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  std::cout << "total time:" << tmp * kSecondToNanosecond / kTestSize
            << std::endl;
  outRes << tmp * kSecondToNanosecond / kTestSize << ",";
}
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADD_H_
