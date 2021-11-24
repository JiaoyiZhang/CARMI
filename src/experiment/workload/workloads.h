/**
 * @file workloads.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef EXPERIMENT_WORKLOAD_WORKLOADS_H_
#define EXPERIMENT_WORKLOAD_WORKLOADS_H_

#include <algorithm>
#include <ctime>
#include <utility>
#include <vector>

#include "../experiment_params.h"
#include "./public_functions.h"
#include "./zipfian.h"

extern std::ofstream outRes;

/**
 * @brief write heavy workload for common CARMI,
 * a mix of 50/50 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] findDataset
 * @param[in] insertDataset
 * @param[inout] carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadA(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMIMap<KeyType, ValueType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kWriteHeavy;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  std::clock_t s, e;
  double tmp;
  auto resIte = carmi->end();
  double res = 0.0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      resIte = carmi->find(findQuery[index[i]].first);
      res += resIte.data();
      carmi->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      resIte = carmi->find(findQuery[i].first);
      res += resIte.data();
      carmi->insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "        res: " << res << std::endl;

  PrintAvgTime(tmp);
}

/**
 * @brief read heavy workload for common CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] findDataset
 * @param[in] insertDataset
 * @param[inout] carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadB(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMIMap<KeyType, ValueType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  std::clock_t s, e;
  double tmp;
  auto resIte = carmi->end();
  double res = 0.0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        resIte = carmi->find(findQuery[index[findCnt]].first);
        res += resIte.data();
        findCnt++;
      }
      carmi->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        resIte = carmi->find(findQuery[findCnt++].first);
        res += resIte.data();
      }
      carmi->insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "        res: " << res << std::endl;

  PrintAvgTime(tmp);
}

/**
 * @brief read only workload for common CARMI, 100% read
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] findDataset
 * @param[inout] carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadC(bool isZipfian, const DataVecType &findDataset,
               CARMIMap<KeyType, ValueType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kReadOnly;
  InitTestSet(findDataset, DataVecType(), &findQuery, &insertQuery, &index);

  std::clock_t s, e;
  double tmp;
  auto resIte = carmi->end();
  double res = 0.0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      resIte = carmi->find(findQuery[index[i]].first);
      res += resIte.data();
    }
  } else {
    for (int i = 0; i < end; i++) {
      resIte = carmi->find(findQuery[i].first);
      res += resIte.data();
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "        res: " << res << std::endl;

  PrintAvgTime(tmp);
}

/**
 * @brief write partial workload for common CARMI
 * a mix of 85/15 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] findDataset
 * @param[in] insertDataset
 * @param[inout] carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadD(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMIMap<KeyType, ValueType> *carmi) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(findDataset, insertDataset, &findQuery, &insertQuery, &index);

  int length = round(kTestSize * kWritePartial);
  int insert_length = round(kTestSize * (1 - kWritePartial));

  int findCnt = 0, insertCnt = 0;

  std::clock_t s, e;
  double tmp;
  double res = 0.0;
  auto resIte = carmi->end();
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        resIte = carmi->find(findQuery[index[findCnt]].first);
        res += resIte.data();
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        carmi->insert(insertQuery[insertCnt]);
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        resIte = carmi->find(findQuery[findCnt].first);
        res += resIte.data();
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        carmi->insert(insertQuery[insertCnt]);
        insertCnt++;
      }
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "        res: " << res << std::endl;

  PrintAvgTime(tmp);
}

/**
 * @brief read mostly workload (range scan) for common CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] findDataset
 * @param[in] insertDataset
 * @param[in] length
 * @param[inout] carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadE(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset, const std::vector<int> &length,
               CARMIMap<KeyType, ValueType> *carmi) {
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
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        auto it = carmi->find(findQuery[index[findCnt]].first);

        for (int l = 0; l < length[index[findCnt]] && it != carmi->end(); l++) {
          ret[l] = {it.key(), it.data()};
          ++it;
        }
        findCnt++;
      }
      carmi->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = carmi->find(findQuery[findCnt].first);
        for (int l = 0; l < length[findCnt] && it != carmi->end(); l++) {
          ret[l] = {it.key(), it.data()};
          ++it;
        }
        findCnt++;
      }
      carmi->insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        typename CARMIMap<KeyType, ValueType>::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename CARMIMap<KeyType, ValueType>::iterator it;
        for (int l = 0; l < length[findCnt]; l++) {
        }
        findCnt++;
      }
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  tmp -= tmp0;

  PrintAvgTime(tmp);
}
#endif  // EXPERIMENT_WORKLOAD_WORKLOADS_H_
