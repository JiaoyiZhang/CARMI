/**
 * @file art_tree_baseline.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-25
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef SRC_BASELINE_ART_TREE_BASELINE_H_
#define SRC_BASELINE_ART_TREE_BASELINE_H_

#include <stdio.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <random>

#include "../../art_tree/art.cpp"
#include "../../art_tree/art.h"
#include "../experiment/workload/zipfian.h"
#include "../params.h"
#include "iostream"

extern std::ofstream outRes;

void artTree_test(double initRatio, const DataVectorType &findDataset,
                  const DataVectorType &insertDataset,
                  const std::vector<int> &length) {
  outRes << "artTree,";
  std::cout << "artTree,";
  auto findData = findDataset;
  auto insertData = insertDataset;
  art_tree t;
  art_tree_init(&t);
  std::cout << "start" << std::endl;
  for (int i = 0; i < findData.size(); i++) {
    char key[64] = {0};
    sprintf(key, "%f", findData[i].first);
    art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1,
               findData[i].second);
  }
  std::cout << "init over" << std::endl;

  std::default_random_engine engine;

  if (initRatio != 2) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed);
    shuffle(findData.begin(), findData.end(), engine);

    unsigned seed1 =
        std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed1);
    shuffle(insertData.begin(), insertData.end(), engine);
  }

  Zipfian zipFind;
  zipFind.InitZipfian(PARAM_ZIPFIAN, findData.size());
  std::vector<int> index;
  for (int i = 0; i < findData.size(); i++) {
    int idx = zipFind.GenerateNextIndex();
    index.push_back(idx);
  }

  if (initRatio == 0.5) {
    int end = 50000;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[index[i]].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[i].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
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
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[index[i]].first);
        sprintf(key, "%f", insertData[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[i].first);
        sprintf(key, "%f", insertData[i].first);
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
  } else if (initRatio == 0.95) {
    int end = 5000;
    int findCnt = 0;

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
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
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
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
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[index[i]].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[i].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
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
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[index[i]].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findData[i].first);
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
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertData[insertCnt].first);
          art_insert(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1,
                     insertData[insertCnt].second);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertData[insertCnt].first);
          art_insert(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1,
                     insertData[insertCnt].second);
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
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertData[insertCnt].first);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 17 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertData.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertData[insertCnt].first);
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

    for (int i = 0; i < findData.size(); i++) {
      int len = std::min(i + length[i], static_cast<int>(findData.size() - 1));
      findData[i].second = findData[len].first;
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed);
    shuffle(findData.begin(), findData.end(), engine);

    unsigned seed1 =
        std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed1);
    shuffle(insertData.begin(), insertData.end(), engine);

    std::chrono::_V2::system_clock::time_point s, e;
    double tmp;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findData[index[findCnt]].second);

          art_range_scan(
              &t, (const unsigned char *)key, strlen((const char *)key) + 1,
              (const unsigned char *)rightKey,
              strlen((const char *)rightKey) + 1, rets, length[index[findCnt]]);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findData[findCnt].second);

          art_range_scan(
              &t, (const unsigned char *)key, strlen((const char *)key) + 1,
              (const unsigned char *)rightKey,
              strlen((const char *)rightKey) + 1, rets, length[findCnt]);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertData[i].second);
      }
    }
    e = std::chrono::system_clock::now();
    tmp = double(std::chrono::duration_cast<std::chrono::nanoseconds>(e - s)
                     .count()) /
          std::chrono::nanoseconds::period::den;

    findCnt = 0;
    s = std::chrono::system_clock::now();
    if (kZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[index[findCnt]].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findData[index[findCnt]].second);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findData.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findData[findCnt].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findData[findCnt].second);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertData[i].first);
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

  outRes << std::endl;
}

#endif  // SRC_BASELINE_ART_TREE_BASELINE_H_
