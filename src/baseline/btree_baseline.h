/**
 * @file btree_baseline.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-25
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_BASELINE_BTREE_BASELINE_H_
#define SRC_BASELINE_BTREE_BASELINE_H_

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "../../stx_btree/btree_map.h"
#include "../experiment/functions.h"
#include "../experiment/workload/zipfian.h"
#include "../params.h"

extern std::ofstream outRes;

void btree_test(bool isZipfian, double initRatio,
                const carmi_params::TestDataVecType &findDataset,
                const carmi_params::TestDataVecType &insertDataset,
                const std::vector<int> &length) {
  std::cout << "btree,";
  outRes << "btree,";
  stx::btree_map<double, double> btree(findDataset.begin(), findDataset.end());

  auto stat = btree.get_stats();
  std::cout << "btree : innernodes" << stat.innernodes
            << "，\tleaf nodes:" << stat.leaves << std::endl;
  double space =
      static_cast<double>(stat.innernodes * 272 + stat.leaves * 280) / 1024 /
      1024;
  std::cout << "btree space:" << space << std::endl;
  outRes << space << std::endl;

  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  double tmp;

  if (initRatio == carmi_params::kWriteHeavy) {
    int end = carmi_params::kTestSize * carmi_params::kWriteHeavy;
    InitTestSet(carmi_params::kWriteHeavy, findDataset, insertDataset,
                &findQuery, &insertQuery, &index);

    std::chrono::_V2::system_clock::time_point s, e;

    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[index[i]].first);
        btree.insert(insertQuery[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[i].first);
        btree.insert(insertQuery[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
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
            std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                .count()) /
        std::chrono::nanoseconds::period::den;
    tmp -= tmp0;

  } else if (initRatio == carmi_params::kReadHeavy) {
    int end = round(carmi_params::kTestSize * (1 - carmi_params::kReadHeavy));
    int findCnt = 0;

    InitTestSet(carmi_params::kReadHeavy, findDataset, insertDataset,
                &findQuery, &insertQuery, &index);

    std::chrono::_V2::system_clock::time_point s, e;

    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          btree.find(findQuery[index[findCnt]].first);
          findCnt++;
        }
        btree.insert(insertQuery[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          btree.find(findQuery[findCnt].first);
          findCnt++;
        }
        btree.insert(insertQuery[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
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
    e = std::chrono::system_clock::now();
    double tmp0 =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                .count()) /
        std::chrono::nanoseconds::period::den;
    tmp -= tmp0;

  } else if (initRatio == carmi_params::kReadOnly) {
    int end = carmi_params::kTestSize * carmi_params::kReadOnly;
    InitTestSet(carmi_params::kReadOnly, findDataset,
                carmi_params::TestDataVecType(), &findQuery, &insertQuery,
                &index);

    std::chrono::_V2::system_clock::time_point s, e;

    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[index[i]].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[i].first);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
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
            std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                .count()) /
        std::chrono::nanoseconds::period::den;
    tmp -= tmp0;
  } else if (initRatio == carmi_params::kWritePartial) {
    int length = round(carmi_params::kTestSize * carmi_params::kWritePartial);
    int insert_length =
        round(carmi_params::kTestSize * (1 - carmi_params::kWritePartial));
    InitTestSet(carmi_params::kWritePartial, findDataset, insertDataset,
                &findQuery, &insertQuery, &index);

    int findCnt = 0, insertCnt = 0;

    std::chrono::_V2::system_clock::time_point s, e;

    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          btree.find(findQuery[index[findCnt]].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          btree.insert(insertQuery[insertCnt]);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          btree.find(findQuery[findCnt].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          btree.insert(insertQuery[insertCnt]);
          insertCnt++;
        }
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    insertCnt = 0;
    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          insertCnt++;
        }
      }
    }
    e = std::chrono::system_clock::now();
    double tmp0 =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                .count()) /
        std::chrono::nanoseconds::period::den;
    tmp -= tmp0;
  } else if (initRatio == carmi_params::kRangeScan) {
    int end = round(carmi_params::kTestSize * (1 - carmi_params::kReadHeavy));
    int findCnt = 0;
    InitTestSet(carmi_params::kReadHeavy, findDataset, insertDataset,
                &findQuery, &insertQuery, &index);

    std::chrono::_V2::system_clock::time_point s, e;

    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          carmi_params::TestDataVecType ret(length[index[findCnt]], {-1, -1});
          auto it = btree.find(findQuery[index[findCnt]].first);
          for (int l = 0; l < length[index[findCnt]]; l++) {
            ret[l] = {it->first, it->second};
            it++;
          }
          findCnt++;
        }
        btree.insert(insertQuery[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          carmi_params::TestDataVecType ret(length[findCnt], {-1, -1});
          auto it = btree.find(findQuery[findCnt].first);
          for (int l = 0; l < length[findCnt]; l++) {
            ret[l] = {it->first, it->second};
            it++;
          }
          findCnt++;
        }
        btree.insert(insertQuery[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    s = std::chrono::system_clock::now();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          carmi_params::TestDataVecType ret(length[index[findCnt]], {-1, -1});
          stx::btree<double, double>::iterator it;
          for (int l = 0; l < length[index[findCnt]]; l++) {
          }
          findCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          carmi_params::TestDataVecType ret(length[findCnt], {-1, -1});
          stx::btree<double, double>::iterator it;
          for (int l = 0; l < length[findCnt]; l++) {
          }
          findCnt++;
        }
      }
    }
    e = std::chrono::system_clock::now();
    double tmp0 =
        static_cast<double>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                .count()) /
        std::chrono::nanoseconds::period::den;
    tmp -= tmp0;
  }

  PrintAvgTime(tmp);
}

#endif  // SRC_BASELINE_BTREE_BASELINE_H_
