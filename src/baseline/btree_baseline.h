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
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "../../stx_btree/btree_map.h"
#include "../experiment/experiment_params.h"
#include "../experiment/functions.h"
#include "../experiment/workload/zipfian.h"

extern std::ofstream outRes;

void btree_test(bool isZipfian, double initRatio,
                const DataVecType &findDataset,
                const DataVecType &insertDataset,
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

  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  double tmp;

  if (initRatio == kWriteHeavy) {
    int end = kTestSize * kWriteHeavy;
    InitTestSet(kWriteHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;

    s = std::clock();
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
    double tmp0 =
        (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;

  } else if (initRatio == kReadHeavy) {
    int end = round(kTestSize * (1 - kReadHeavy));
    int findCnt = 0;

    InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;

    s = std::clock();
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
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    findCnt = 0;
    s = std::clock();
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
    e = std::clock();
    double tmp0 =
        (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;

  } else if (initRatio == kReadOnly) {
    int end = kTestSize * kReadOnly;
    InitTestSet(kReadOnly, findDataset, DataVecType(), &findQuery, &insertQuery,
                &index);

    std::clock_t s, e;

    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[index[i]].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        btree.find(findQuery[i].first);
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
    double tmp0 =
        (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kWritePartial) {
    int length = round(kTestSize * kWritePartial);
    int insert_length = round(kTestSize * (1 - kWritePartial));
    InitTestSet(kWritePartial, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    int findCnt = 0, insertCnt = 0;

    std::clock_t s, e;

    s = std::clock();
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
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    findCnt = 0;
    insertCnt = 0;
    s = std::clock();
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
    e = std::clock();
    double tmp0 =
        (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kRangeScan) {
    int end = round(kTestSize * (1 - kReadHeavy));
    int findCnt = 0;
    InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;

    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          DataVecType ret(length[index[findCnt]], {-1, -1});
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
          DataVecType ret(length[findCnt], {-1, -1});
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
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    findCnt = 0;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          DataVecType ret(length[index[findCnt]], {-1, -1});
          stx::btree<double, double>::iterator it;
          for (int l = 0; l < length[index[findCnt]]; l++) {
          }
          findCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          DataVecType ret(length[findCnt], {-1, -1});
          stx::btree<double, double>::iterator it;
          for (int l = 0; l < length[findCnt]; l++) {
          }
          findCnt++;
        }
      }
    }
    e = std::clock();
    double tmp0 =
        (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  }

  PrintAvgTime(tmp);
}

#endif  // SRC_BASELINE_BTREE_BASELINE_H_
