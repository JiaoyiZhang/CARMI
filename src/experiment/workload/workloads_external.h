/**
 * @file workloads_external.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_

#include <chrono>
#include <utility>
#include <vector>

#include "../../include/carmi_external.h"
#include "../../include/func/find_function.h"
#include "../../include/func/insert_function.h"
#include "../functions.h"
#include "./public_functions.h"
#include "./zipfian.h"

extern std::ofstream outRes;

/**
 * @brief write heavy workload for external CARMI,
 * a mix of 50/50 reads and writes
 *
 * @tparam KeyType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param carmi
 */
template <typename KeyType>
void WorkloadA(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               CARMIExternal<KeyType> *carmi) {
  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  int end = carmi_params::kTestSize * carmi_params::kWriteHeavy;
  InitTestSet(carmi_params::kWriteHeavy, findDataset, insertDataset, &findQuery,
              &insertQuery, &index);

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[index[i]].first);
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  } else {
    for (int i = 0; i < end; i++) {
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  }
  e = std::chrono::system_clock::now();
  double tmp0 =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

/**
 * @brief read heavy workload for external CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param carmi
 */
template <typename KeyType>
void WorkloadB(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               CARMIExternal<KeyType> *carmi) {
  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(carmi_params::kReadHeavy, findDataset, insertDataset, &findQuery,
              &insertQuery, &index);

  int end = round(carmi_params::kTestSize * (1 - carmi_params::kReadHeavy));
  int findCnt = 0;

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) {
        carmi->Find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        carmi->Find(findQuery[findCnt++].first);
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) findCnt++;
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) findCnt++;
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  }
  e = std::chrono::system_clock::now();
  double tmp0 =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

/**
 * @brief read only workload for external CARMI, 100% read
 *
 * @tparam KeyType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param carmi
 */
template <typename KeyType>
void WorkloadC(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               CARMIExternal<KeyType> *carmi) {
  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  int end = carmi_params::kTestSize * carmi_params::kReadOnly;
  InitTestSet(carmi_params::kReadOnly, findDataset,
              carmi_params::TestDataVecType(), &findQuery, &insertQuery,
              &index);

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[index[i]].first);
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
    }
  } else {
    for (int i = 0; i < end; i++) {
    }
  }
  e = std::chrono::system_clock::now();
  double tmp0 =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

/**
 * @brief read mostly workload (range scan) for external CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param length
 * @param carmi
 */
template <typename KeyType>
void WorkloadE(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               const std::vector<int> &length, CARMIExternal<KeyType> *carmi) {
  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(carmi_params::kReadHeavy, findDataset, insertDataset, &findQuery,
              &insertQuery, &index);

  int end = round(carmi_params::kTestSize * (1 - carmi_params::kReadHeavy));
  int findCnt = 0;

  carmi_params::TestDataVecType ret(100, {-1, -1});
  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        auto it = carmi->Find(findQuery[index[findCnt]].first);

        for (int l = 0; l < length[index[findCnt]] && it != it.end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = carmi->Find(findQuery[findCnt].first);
        for (int l = 0; l < length[findCnt] && it != it.end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        typename CARMIExternal<KeyType>::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename CARMIExternal<KeyType>::iterator it;
        for (int l = 0; l < length[findCnt]; l++) {
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  }
  e = std::chrono::system_clock::now();
  double tmp0 =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_
