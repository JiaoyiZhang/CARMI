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
#include "../experiment/workload/zipfian.h"
#include "../params.h"

extern std::ofstream outRes;

void btree_test(double initRatio, const DataVectorType &findDataset,
                const DataVectorType &insertDataset,
                const std::vector<int> &length) {
  std::cout << "btree,";
  outRes << "btree,";
  auto findData = findDataset;
  auto insertData = insertDataset;
  stx::btree_map<double, double> btree(findData.begin(), findData.end());

  auto stat = btree.get_stats();
  std::cout << "btree : innernodes" << stat.innernodes
            << "，\tleaf nodes:" << stat.leaves << std::endl;
  double space =
      static_cast<double>(stat.innernodes * 272 + stat.leaves * 280) / 1024 /
      1024;
  std::cout << "btree space:" << space << std::endl;
  outRes << space << std::endl;

  std::default_random_engine engine;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed);
  shuffle(findData.begin(), findData.end(), engine);

  unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed1);
  shuffle(insertData.begin(), insertData.end(), engine);

  Zipfian zipFind;
  zipFind.InitZipfian(PARAM_ZIPFIAN, findData.size());
  std::vector<int> index;
  for (int i = 0; i < findData.size(); i++) {
    int idx = zipFind.GenerateNextIndex();
    index.push_back(idx);
  }

  if (initRatio == kWriteHeavy) {
    int end = kTestSize * kWriteHeavy;  // 50000
    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        btree.find(findData[index[i]].first);
        btree.insert(insertData[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        btree.find(findData[i].first);
        btree.insert(insertData[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    s = std::chrono::system_clock::now();
    if (kZipfian) {
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

    std::cout << "total time:" << tmp * kSecondToNanosecond / kTestSize
              << std::endl;
    outRes << tmp * kSecondToNanosecond / kTestSize << ",";

  } else if (initRatio == 0.95) {
    int end = 5000;
    int findCnt = 0;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          btree.find(findData[index[findCnt]].first);
          findCnt++;
        }
        btree.insert(insertData[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          btree.find(findData[findCnt].first);
          findCnt++;
        }
        btree.insert(insertData[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          findCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
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

    std::cout << "total time:" << tmp / 100000.0 * 1000000000 << std::endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
  } else if (initRatio == 1) {
    int end = 100000;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        btree.find(findData[index[i]].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        btree.find(findData[i].first);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    s = std::chrono::system_clock::now();
    if (kZipfian) {
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

    std::cout << "total time:" << tmp / 100000.0 * 1000000000 << std::endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
  } else if (initRatio == 0) {
    int end = 5000;
    int findCnt = 0;
    int insertCnt = 0;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          btree.find(findData[index[findCnt]].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          btree.insert(insertData[insertCnt]);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          btree.find(findData[findCnt].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          btree.insert(insertData[insertCnt]);
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
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
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

    std::cout << "total time:" << tmp / 100000.0 * 1000000000 << std::endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
  } else if (initRatio == 2) {
    int end = 5000;
    int findCnt = 0;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          DataVectorType ret(length[index[findCnt]], {-1, -1});
          auto it = btree.find(findData[index[findCnt]].first);
          for (int l = 0; l < length[index[findCnt]]; l++) {
            ret[l] = {it->first, it->second};
            it++;
          }
          findCnt++;
        }
        btree.insert(insertData[i]);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          DataVectorType ret(length[findCnt], {-1, -1});
          auto it = btree.find(findData[findCnt].first);
          for (int l = 0; l < length[findCnt]; l++) {
            ret[l] = {it->first, it->second};
            it++;
          }
          findCnt++;
        }
        btree.insert(insertData[i]);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = static_cast<double>(
              std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                  .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          DataVectorType ret(length[index[findCnt]], {-1, -1});
          stx::btree<double, double>::iterator it;
          for (int l = 0; l < length[index[findCnt]]; l++) {
          }
          findCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          DataVectorType ret(length[findCnt], {-1, -1});
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

    std::cout << "total time:" << tmp / 100000.0 * 1000000000 << std::endl;
    outRes << tmp / 100000.0 * 1000000000 << ",";
  }
}

#endif  // SRC_BASELINE_BTREE_BASELINE_H_
