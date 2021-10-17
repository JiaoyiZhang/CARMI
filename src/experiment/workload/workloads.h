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

#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_

#include <algorithm>
#include <ctime>
#include <utility>
#include <vector>

#include "../../include/carmi_common.h"
#include "../../include/func/find_function.h"
#include "../../include/func/insert_function.h"
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
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadA(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMICommon<KeyType, ValueType> *carmi) {
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
      carmi->Insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
      carmi->Insert(insertQuery[i]);
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
 * @brief read heavy workload for common CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadB(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMICommon<KeyType, ValueType> *carmi) {
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
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        carmi->Find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      carmi->Insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        carmi->Find(findQuery[findCnt++].first);
      }
      carmi->Insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) {
        findCnt++;
      }
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        findCnt++;
      }
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

/**
 * @brief read only workload for common CARMI, 100% read
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadC(bool isZipfian, const DataVecType &findDataset,
               CARMICommon<KeyType, ValueType> *carmi) {
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
 * @brief write partial workload for common CARMI
 * a mix of 85/15 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadD(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               CARMICommon<KeyType, ValueType> *carmi) {
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
        resIte = carmi->Find(findQuery[index[findCnt]].first);
        res += resIte.data();
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        carmi->Insert(insertQuery[insertCnt]);
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        carmi->Find(findQuery[findCnt].first);
        res += resIte.data();
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        carmi->Insert(insertQuery[insertCnt]);
        insertCnt++;
      }
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  insertCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) findCnt++;
      for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        insertCnt++;
      }
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

/**
 * @brief read mostly workload (range scan) for common CARMI,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param length
 * @param carmi
 */
template <typename KeyType, typename ValueType>
void WorkloadE(bool isZipfian, const DataVecType &findDataset,
               const DataVecType &insertDataset, const std::vector<int> &length,
               CARMICommon<KeyType, ValueType> *carmi) {
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
        auto it = carmi->Find(findQuery[index[findCnt]].first);

        for (int l = 0; l < length[index[findCnt]] && it != carmi->end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      carmi->Insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = carmi->Find(findQuery[findCnt].first);
        for (int l = 0; l < length[findCnt] && it != carmi->end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      carmi->Insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        typename CARMICommon<KeyType, ValueType>::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename CARMICommon<KeyType, ValueType>::iterator it;
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
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_
