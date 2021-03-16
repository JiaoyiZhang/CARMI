/**
 * @file workloadc.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADC_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADC_H_
#include <vector>

#include "../../CARMI/carmi.h"
#include "../../CARMI/func/find_function.h"
#include "../../CARMI/func/insert_function.h"
#include "./zipfian.h"
extern ofstream outRes;

// read only workload
// 100% read
void WorkloadC(const DataVectorType& initDataset, CARMI* carmi) {
  auto init = initDataset;

  default_random_engine engine;

  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  engine = default_random_engine(seed);
  shuffle(init.begin(), init.end(), engine);

  int end = kTestSize;
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, init.size());
  vector<int> index(end, 0);
  for (int i = 0; i < end; i++) {
    int idx = zip.GenerateNextIndex();
    index[i] = idx;
  }

  chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = chrono::system_clock::now();
#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    carmi->Find(init[index[i]].first);
  }
#else
  for (int i = 0; i < end; i++) {
    carmi->Find(init[i].first);
  }
#endif
  e = chrono::system_clock::now();
  tmp = static_cast<double>(
            chrono::duration_cast<chrono::nanoseconds>(e - s).count()) /
        chrono::nanoseconds::period::den;

  s = chrono::system_clock::now();

#if ZIPFIAN
  for (int i = 0; i < end; i++) {
    // init[index[i]].first;
  }
#else
  for (int i = 0; i < end; i++) {
    // init[i].first;
  }
#endif
  e = chrono::system_clock::now();
  double tmp0 = static_cast<double>(
                    chrono::duration_cast<chrono::nanoseconds>(e - s).count()) /
                chrono::nanoseconds::period::den;
  tmp -= tmp0;

  cout << "total time:" << tmp * kSecondToNanosecond / kTestSize << endl;
  outRes << tmp * kSecondToNanosecond / kTestSize << ",";
}

#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADC_H_
