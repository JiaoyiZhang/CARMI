/**
 * @file workloadb.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADB_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADB_H_
#include <chrono>
#include <vector>

#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

// read heavy workload
// a mix of 95/5 reads and writes
void WorkloadB(const DataVectorType &initDataset,
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

  int end = round(kTestSize * (1 - kReadHeavy));
#ifdef DEBUG
  if (end != 5000)
    std::cout << "wrong, workload b, length:" << end << std::endl;
#endif  // DEBUG
  int findCnt = 0;
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, init.size());
  std::vector<int> index(kTestSize * kReadHeavy, 0);
  for (int i = 0; i < kTestSize * kReadHeavy; i++) {
    int idx = zip.GenerateNextIndex();
    index[i] = idx;
  }

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19; j++) {
      carmi->Find(init[index[findCnt]].first);
      findCnt++;
    }
    carmi->Insert(insert[i]);
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      carmi->Find(init[findCnt].first);
      findCnt++;
    }
    carmi->Insert(insert[i]);
  }
#endif
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19; j++) {
      findCnt++;
    }
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      findCnt++;
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

#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADB_H_
