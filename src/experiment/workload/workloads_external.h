/**
 * @file workloads_external.h
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_

#include <ctime>
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
void WorkloadA(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMIExternal<KeyType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kWriteHeavy;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[index[i]].first);
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  s = std::clock();
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
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
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
void WorkloadB(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMIExternal<KeyType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) {
        carmi->Find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(findQuery.size());
           j++) {
        carmi->Find(findQuery[findCnt++].first);
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) findCnt++;
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(findQuery.size());
           j++)
        findCnt++;
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
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
void WorkloadC(bool isZipfian, const DataVecType &findDataset,
               CARMIExternal<KeyType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kReadOnly;
  InitTestSet(findDataset, DataVecType(), &findQuery, &insertQuery, &index);

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[index[i]].first);
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
    }
  } else {
    for (int i = 0; i < end; i++) {
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
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
void WorkloadE(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset, const std::vector<int> &length,
               CARMIExternal<KeyType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  DataVecType ret(100, {-1, -1});
  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(index.size()); j++) {
        auto it = static_cast<const KeyType *>(
            carmi->Find(findQuery[index[findCnt]].first));

        for (int l = 0; l < length[index[findCnt]]; l++) {
          ret[l] = {*it, *it};
          it++;
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(findQuery.size());
           j++) {
        auto it =
            static_cast<const KeyType *>(carmi->Find(findQuery[findCnt].first));
        for (int l = 0; l < length[findCnt]; l++) {
          ret[l] = {*it, *it};
          it++;
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data.first);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(index.size()); j++) {
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < static_cast<int>(findQuery.size());
           j++) {
        for (int l = 0; l < length[findCnt]; l++) {
        }
        findCnt++;
      }
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  tmp -= tmp0;

  PrintAvgTime(tmp);
}
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADS_EXTERNAL_H_
