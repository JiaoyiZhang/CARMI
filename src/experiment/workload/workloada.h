/**
 * @file workloada.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADA_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADA_H_
#include <chrono>
#include <vector>

#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

// write heavy workload
// a mix of 50/50 reads and writes
void WorkloadA(const DataVectorType &initDataset,
               const DataVectorType &insertDataset, CARMI *carmi) {
  DataVectorType init = initDataset;
  DataVectorType insert = insertDataset;

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

  int end = kTestSize * kWriteHeavy;  // 50000
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, init.size());
  std::vector<int> index(end, 0);
  for (int i = 0; i < end; i++) {
    int idx = zip.GenerateNextIndex();
    index[i] = idx;
  }

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    carmi->Find(init[index[i]].first);
    carmi->Insert(insert[i]);
  }
#else
  for (int i = 0; i < end; i++) {
    carmi->Find(init[i].first);
    carmi->Insert(insert[i]);
  }
#endif
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
  }
#else
  for (int i = 0; i < end; i++) {
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

#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADA_H_
