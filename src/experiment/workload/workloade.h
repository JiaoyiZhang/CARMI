
/**
 * @file workloade.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADE_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADE_H_
#include <chrono>
#include <vector>

#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

// read mostly workload (range scan)
// a mix of 95/5 reads and writes
void WorkloadE(const DataVectorType &initDataset,
               const DataVectorType &insertDataset,
               const std::vector<int> &length, CARMI *carmi) {
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

  int end = round(kTestSize * (1 - kRangeScan));
#ifdef DEBUG
  if (end != 5000)
    std::cout << "wrong, workload e, length:" << end << std::endl;
#endif  // DEBUG
  int findCnt = 0;
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, init.size());
  int tmp_end = round(kTestSize * kRangeScan);
#ifdef DEBUG
  if (tmp_end != 5000)
    std::cout << "wrong, workload e, tmp_end:" << tmp_end << std::endl;
#endif  // DEBUG
  std::vector<int> index(tmp_end, 0);
  for (int i = 0; i < tmp_end; i++) {
    int idx = zip.GenerateNextIndex();
    index[i] = idx;
  }

  DataVectorType ret(100, {-1, -1});
  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      auto it = carmi->Find(init[index[findCnt]].first);
      for (int l = 0; l < length[index[findCnt]] && it != it.end(); l++) {
        ret[l] = {it.key(), it.data()};
        ++it;
      }
      findCnt++;
    }
    carmi->Insert(insert[i]);
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      auto it = carmi->Find(init[findCnt].first);
      for (int l = 0; l < length[findCnt] && it != it.end(); l++) {
        ret[l] = {it.key(), it.data()};
        ++it;
      }
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
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      CARMI::iterator it;
      for (int l = 0; l < length[index[findCnt]]; l++) {
      }
      findCnt++;
    }
  }
#else
  for (int i = 0; i < end; i++) {
    for (int j = 0; j < 19 && findCnt < init.size(); j++) {
      CARMI::iterator it;
      for (int l = 0; l < length[findCnt]; l++) {
      }
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

#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADE_H_
