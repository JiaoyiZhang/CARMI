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
#include <fstream>
#include <random>

#include "../../art_tree/art.cpp"
#include "../../art_tree/art.h"
#include "../experiment/experiment_params.h"
#include "../experiment/functions.h"
#include "../experiment/workload/zipfian.h"
#include "iostream"

extern std::ofstream outRes;

void artTree_test(bool isZipfian, double initRatio,
                  const DataVecType &findDataset,
                  const DataVecType &insertDataset,
                  const std::vector<int> &length) {
  outRes << "artTree,";
  std::cout << "artTree,";
  art_tree t;
  art_tree_init(&t);
  std::cout << "start" << std::endl;
  for (int i = 0; i < findDataset.size(); i++) {
    char key[64] = {0};
    sprintf(key, "%f", findDataset[i].first);
    art_insert(&t, (const unsigned char *)key, strlen((const char *)key) + 1,
               findDataset[i].second);
  }
  std::cout << "init over" << std::endl;

  DataVecType findQuery;
  DataVecType insertQuery;
  std::vector<int> index;
  double tmp;

  if (initRatio == kWriteHeavy) {
    int end = kTestSize * kWriteHeavy;
    InitTestSet(kWriteHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[index[i]].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[i].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    }
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[index[i]].first);
        sprintf(key, "%f", insertQuery[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[i].first);
        sprintf(key, "%f", insertQuery[i].first);
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kReadHeavy) {
    int end = round(kTestSize * (1 - kReadHeavy));
    int findCnt = 0;

    InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    }
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    findCnt = 0;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kReadOnly) {
    int end = kTestSize * kReadOnly;
    InitTestSet(kReadOnly, findDataset, DataVecType(), &findQuery, &insertQuery,
                &index);

    std::clock_t s, e;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[index[i]].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[i].first);
        art_search(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, rets);
      }
    }
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[index[i]].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        std::vector<double> rets;
        char key[64] = {0};
        sprintf(key, "%f", findQuery[i].first);
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kWritePartial) {
    int length = round(kTestSize * kWritePartial);
    int insert_length = round(kTestSize * (1 - kWritePartial));
    InitTestSet(kWritePartial, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    int findCnt = 0, insertCnt = 0;

    std::clock_t s, e;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertQuery[insertCnt].first);
          art_insert(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1,
                     insertQuery[insertCnt].second);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          art_search(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1, rets);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertQuery[insertCnt].first);
          art_insert(&t, (const unsigned char *)key,
                     strlen((const char *)key) + 1,
                     insertQuery[insertCnt].second);
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
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertQuery[insertCnt].first);
          insertCnt++;
        }
      }
    } else {
      for (int i = 0; i < insert_length; i++) {
        for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          findCnt++;
        }
        for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
          char key[64] = {0};
          sprintf(key, "%f", insertQuery[insertCnt].first);
          insertCnt++;
        }
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  } else if (initRatio == kRangeScan) {
    int end = round(kTestSize * (1 - kReadHeavy));
    int findCnt = 0;
    for (int i = 0; i < findQuery.size(); i++) {
      int len = std::min(i + length[i], static_cast<int>(findQuery.size() - 1));
      findQuery[i].second = findQuery[len].first;
    }
    InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery,
                &insertQuery, &index);

    std::clock_t s, e;
    double tmp;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findQuery[index[findCnt]].second);

          art_range_scan(
              &t, (const unsigned char *)key, strlen((const char *)key) + 1,
              (const unsigned char *)rightKey,
              strlen((const char *)rightKey) + 1, rets, length[index[findCnt]]);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findQuery[findCnt].second);

          art_range_scan(
              &t, (const unsigned char *)key, strlen((const char *)key) + 1,
              (const unsigned char *)rightKey,
              strlen((const char *)rightKey) + 1, rets, length[findCnt]);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
        art_insert(&t, (const unsigned char *)key,
                   strlen((const char *)key) + 1, insertQuery[i].second);
      }
    }
    e = std::clock();
    tmp = (e - s) / static_cast<double>(CLOCKS_PER_SEC);

    findCnt = 0;
    s = std::clock();
    if (isZipfian) {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[index[findCnt]].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findQuery[index[findCnt]].second);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
      }
    } else {
      for (int i = 0; i < end; i++) {
        for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
          std::vector<double> rets;
          char key[64] = {0};
          sprintf(key, "%f", findQuery[findCnt].first);
          char rightKey[64] = {0};
          sprintf(rightKey, "%f", findQuery[findCnt].second);
          findCnt++;
        }
        char key[64] = {0};
        sprintf(key, "%f", insertQuery[i].first);
      }
    }
    e = std::clock();
    double tmp0 = (e - s) / static_cast<double>(CLOCKS_PER_SEC);
    tmp -= tmp0;
  }

  PrintAvgTime(tmp);

  outRes << std::endl;
}

#endif  // SRC_BASELINE_ART_TREE_BASELINE_H_
