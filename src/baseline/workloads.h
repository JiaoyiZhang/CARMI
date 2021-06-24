/**
 * @file workloads.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-06-08
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_BASELINE_WORKLOADS_H_
#define SRC_BASELINE_WORKLOADS_H_

#include <algorithm>
#include <ctime>
#include <utility>
#include <vector>

#include "../../alex/alex_map.h"
#include "../../rs/multi_map.h"
#include "../../stx_btree/btree_map.h"
#include "../experiment/experiment_params.h"
#include "../experiment/functions.h"
#include "../experiment/workload/zipfian.h"

extern std::ofstream outRes;

/**
 * @brief write heavy workload for  baselineIndex,
 * a mix of 50/50 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param baselineIndex
 */
template <typename BaselineType>
void BaselineWorkloadA(bool isZipfian, const DataVecType &findDataset,
                       const DataVecType &insertDataset,
                       BaselineType *baselineIndex) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kWriteHeavy;
  InitTestSet(kWriteHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      baselineIndex->find(findQuery[index[i]].first);
      baselineIndex->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      baselineIndex->find(findQuery[i].first);
      baselineIndex->insert(insertQuery[i]);
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
 * @brief read heavy workload for  baselineIndex,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param baselineIndex
 */
template <typename BaselineType>
void BaselineWorkloadB(bool isZipfian, const DataVecType &findDataset,
                       const DataVecType &insertDataset,
                       BaselineType *baselineIndex) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        baselineIndex->find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      baselineIndex->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        baselineIndex->find(findQuery[findCnt++].first);
      }
      baselineIndex->insert(insertQuery[i]);
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
 * @brief read only workload for  baselineIndex, 100% read
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param baselineIndex
 */
template <typename BaselineType>
void BaselineWorkloadC(bool isZipfian, const DataVecType &findDataset,
                       BaselineType *baselineIndex) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kReadOnly;
  InitTestSet(kReadOnly, findDataset, DataVecType(), &findQuery, &insertQuery,
              &index);

  std::clock_t s, e;
  auto resIte = baselineIndex->end();
  double res = 0.0;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      // #ifdef DEBUG
      //       typename BaselineType::iterator it =
      //           baselineIndex->find(findQuery[index[i]].first);
      //       if (it.key() != findQuery[index[i]].first) {
      //         std::cout << "find wrong! key:" << findQuery[index[i]].first
      //                   << ",\tit.key:" << it.key() << std::endl;
      //         baselineIndex->find(findQuery[index[i]].first);
      //       }
      // #else
      resIte = baselineIndex->find(findQuery[index[i]].first);
      res += resIte.key();
      // #endif  // DEBUG
    }
  } else {
    for (int i = 0; i < end; i++) {
      // #ifdef DEBUG
      //       typename BaselineType::iterator it =
      //           baselineIndex->find(findQuery[i].first);
      //       if (it.key() != findQuery[i].first) {
      //         baselineIndex->find(findQuery[i].first);
      //         std::cout << "find wrong! key:" << findQuery[i].first
      //                   << ",\tit.key:" << it.key() << std::endl;
      //       }
      // #else
      resIte = baselineIndex->find(findQuery[i].first);
      res += resIte.key();
      // #endif  // DEBUG
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  std::cout << "        res: " << res << std::endl;
  res = 0;

  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      resIte = baselineIndex->end();
      res += i;
    }
  } else {
    for (int i = 0; i < end; i++) {
      resIte = baselineIndex->end();
      res += i;
    }
  }
  e = std::clock();
  double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
  tmp -= tmp0;

  std::cout << "        res: " << res << std::endl;
  res = 0;
  PrintAvgTime(tmp);
}

/**
 * @brief write partial workload for  baselineIndex
 * a mix of 85/15 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param baselineIndex
 */
template <typename BaselineType>
void BaselineWorkloadD(bool isZipfian, const DataVecType &findDataset,
                       const DataVecType &insertDataset,
                       BaselineType *baselineIndex) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(kWritePartial, findDataset, insertDataset, &findQuery,
              &insertQuery, &index);

  int length = round(kTestSize * kWritePartial);
  int insert_length = round(kTestSize * (1 - kWritePartial));

  int findCnt = 0, insertCnt = 0;

  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        baselineIndex->find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        baselineIndex->insert(insertQuery[insertCnt]);
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        baselineIndex->find(findQuery[findCnt].first);
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
        baselineIndex->insert(insertQuery[insertCnt]);
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
 * @brief read mostly workload (range scan) for  baselineIndex,
 * a mix of 95/5 reads and writes
 *
 * @tparam KeyType
 * @tparam ValueType
 * @param isZipfian whether to use zipfian access during the test
 * @param findDataset
 * @param insertDataset
 * @param length
 * @param baselineIndex
 */
template <typename BaselineType>
void BaselineWorkloadE(bool isZipfian, const DataVecType &findDataset,
                       const DataVecType &insertDataset,
                       const std::vector<int> &length,
                       BaselineType *baselineIndex) {
  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  DataVecType ret(100, {-1, -1});
  std::clock_t s, e;
  double tmp;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        auto it = baselineIndex->find(findQuery[index[findCnt]].first);

        for (int l = 0;
             l < length[index[findCnt]] && it != baselineIndex->end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      baselineIndex->insert(insertQuery[i]);
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = baselineIndex->find(findQuery[findCnt].first);
        for (int l = 0; l < length[findCnt] && it != baselineIndex->end();
             l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
      baselineIndex->insert(insertQuery[i]);
    }
  }
  e = std::clock();
  tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

  findCnt = 0;
  s = std::clock();
  if (isZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < index.size(); j++) {
        typename BaselineType::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename BaselineType::iterator it;
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

void test_btree(bool isZipfian, double initRatio,
                const DataVecType &findDataset,
                const DataVecType &insertDataset,
                const std::vector<int> &length) {
  std::cout << "btree,";
  outRes << "btree,";
  stx::btree_map<double, double> btree(findDataset.begin(), findDataset.end());

  auto stat = btree.get_stats();
  // std::cout << "btree : innernodes" << stat.innernodes
  //           << "，\tleaf nodes:" << stat.leaves << std::endl;
  double space =
      static_cast<double>(stat.innernodes * 272 + stat.leaves * 280) / 1024 /
      1024;
  std::cout << "btree space:" << space <<","<< std::endl;
  outRes << space;

  if (initRatio == kWriteHeavy)
    BaselineWorkloadA<stx::btree_map<double, double>>(isZipfian, findDataset,
                                                      insertDataset,
                                                      &btree);  // write-heavy
  else if (initRatio == kReadHeavy)
    BaselineWorkloadB<stx::btree_map<double, double>>(isZipfian, findDataset,
                                                      insertDataset,
                                                      &btree);  // read-heavy
  else if (initRatio == kReadOnly)
    BaselineWorkloadC<stx::btree_map<double, double>>(isZipfian, findDataset,
                                                      &btree);  // read-only
  else if (initRatio == kWritePartial)
    BaselineWorkloadD<stx::btree_map<double, double>>(isZipfian, findDataset,
                                                      insertDataset,
                                                      &btree);  // write-partial
  else if (initRatio == kRangeScan)
    BaselineWorkloadE<stx::btree_map<double, double>>(isZipfian, findDataset,
                                                      insertDataset, length,
                                                      &btree);  // range scan
}

void test_alex(bool isZipfian, double initRatio, const DataVecType &findDataset,
               const DataVecType &insertDataset,
               const std::vector<int> &length) {
  std::cout << "alex,";
  outRes << "alex,";
  alex::AlexMap<double, double> alex(findDataset.begin(), findDataset.end());

  // auto stat = btree.get_stats();
  // std::cout << "btree : innernodes" << stat.innernodes
  //           << "，\tleaf nodes:" << stat.leaves << std::endl;
  // double space =
  //     static_cast<double>(stat.innernodes * 272 + stat.leaves * 280) / 1024 /
  //     1024;
  // std::cout << "btree space:" << space << std::endl;
  // outRes << space;

  if (initRatio == kWriteHeavy)
    BaselineWorkloadA<alex::AlexMap<double, double>>(isZipfian, findDataset,
                                                     insertDataset,
                                                     &alex);  // write - heavy
  else if (initRatio == kReadHeavy)
    BaselineWorkloadB<alex::AlexMap<double, double>>(isZipfian, findDataset,
                                                     insertDataset,
                                                     &alex);  // read-heavy
  else if (initRatio == kReadOnly)
    BaselineWorkloadC<alex::AlexMap<double, double>>(isZipfian, findDataset,
                                                     &alex);  // read-only
  else if (initRatio == kWritePartial)
    BaselineWorkloadD<alex::AlexMap<double, double>>(isZipfian, findDataset,
                                                     insertDataset,
                                                     &alex);  // write - partial
  else if (initRatio == kRangeScan)
    BaselineWorkloadE<alex::AlexMap<double, double>>(isZipfian, findDataset,
                                                     insertDataset, length,
                                                     &alex);  // range scan
}

void test_radix_spline(bool isZipfian, double initRatio,
                       const DataVecType &findDataset,
                       const DataVecType &insertDataset,
                       const std::vector<int> &length) {
  std::cout << "RadixSpline,";
  outRes << "RadixSpline,";
  rs::MultiMap<uint64_t, int> rs(findDataset.begin(), findDataset.end());

  if (initRatio == kReadOnly) {
    DataVecType findQuery;
    DataVecType insertQuery;
    std::vector<int> index;
    int end = kTestSize * kReadOnly;
    InitTestSet(kReadOnly, findDataset, DataVecType(), &findQuery, &insertQuery,
                &index);

    std::clock_t s, e;
    auto resIte = rs.end();
    double res = 0.0;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        resIte = rs.find(findQuery[index[i]].first);
        res += resIte->second;
      }
    } else {
      for (int i = 0; i < end; i++) {
        resIte = rs.find(findQuery[i].first);
        res += resIte->second;
      }
    }
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    std::cout << "        res: " << res << std::endl;
    res = 0;

    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        resIte = rs.end();
        res += i;
      }
    } else {
      for (int i = 0; i < end; i++) {
        resIte = rs.end();
        res += i;
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;

    std::cout << "        res: " << res << std::endl;
    res = 0;
    PrintAvgTime(tmp);
  } else {
    std::cout << "RadixSpline only supports read-only workloads." << std::endl;
  }
}

#endif  // SRC_BASELINE_WORKLOADS_H_
