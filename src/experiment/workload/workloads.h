/**
 * @file workloads.h
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-03-26
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_
#define SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_

#include <chrono>
#include <utility>
#include <vector>

#include "../../include/carmi.h"
#include "../../include/func/find_function.h"
#include "../../include/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

void InitTestSet(int Ratio, const carmi_params::TestDataVecType &findQueryset,
                 const carmi_params::TestDataVecType &insertDataset,
                 carmi_params::TestDataVecType *findQuery,
                 carmi_params::TestDataVecType *insertQuery,
                 std::vector<int> *index) {
  (*findQuery) = findQueryset;  // -> findQueryset
  (*insertQuery) = insertDataset;

  std::default_random_engine engine;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed);
  shuffle((*findQuery).begin(), (*findQuery).end(), engine);

  if (!carmi_params::kPrimaryIndex) {
    unsigned seed1 =
        std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed1);
    shuffle((*insertQuery).begin(), (*insertQuery).end(), engine);
  }

  int end = round(carmi_params::kTestSize * Ratio);  // 50000
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, (*findQuery).size());
  *index = std::vector<int>(end, 0);
  for (int i = 0; i < end; i++) {
    int idx = zip.GenerateNextIndex();
    (*index)[i] = idx;
  }
}

void PrintAvgTime(double time) {
  std::cout << "total time:"
            << time * carmi_params::kSecondToNanosecond /
                   carmi_params::kTestSize
            << std::endl;
  outRes << time * carmi_params::kSecondToNanosecond / carmi_params::kTestSize
         << ",";
}

// write heavy workload
// a mix of 50/50 reads and writes
template <typename CarmiType, typename KeyType, typename ValueType>
void WorkloadA(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               CarmiType *carmi) {
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
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
    }
  } else {
    for (int i = 0; i < end; i++) {
      carmi->Find(findQuery[i].first);
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
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
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#else
#endif  // EXTERNAL
    }
  } else {
    for (int i = 0; i < end; i++) {
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#else
#endif  // EXTERNAL
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

// read heavy workload
// a mix of 95/5 reads and writes
template <typename CarmiType, typename KeyType, typename ValueType>
void WorkloadB(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               CarmiType *carmi) {
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
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        carmi->Find(findQuery[findCnt++].first);
      }
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
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
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) findCnt++;
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
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

// read only workload
// 100% read
template <typename CarmiType, typename KeyType, typename ValueType>
void WorkloadC(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               CarmiType *carmi) {
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

// write partial workload
// a mix of 85/15 reads and writes
template <typename CarmiType, typename KeyType, typename ValueType>
void WorkloadD(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               CarmiType *carmi) {
  carmi_params::TestDataVecType findQuery;
  carmi_params::TestDataVecType insertQuery;
  std::vector<int> index;
  InitTestSet(carmi_params::kWritePartial, findDataset, insertDataset,
              &findQuery, &insertQuery, &index);

  int length = round(carmi_params::kTestSize * carmi_params::kWritePartial);
  int insert_length =
      round(carmi_params::kTestSize * (1 - carmi_params::kWritePartial));

  int findCnt = 0, insertCnt = 0;

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
        carmi->Find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
#ifdef EXTERNAL
        std::pair<KeyType, std::vector<KeyType>> data = {
            insertQuery[insertCnt].first,
            std::vector<KeyType>(1, insertQuery[i].second)};
        carmi->Insert(data);
#else
        carmi->Insert(insertQuery[insertCnt]);
#endif  // EXTERNAL
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        carmi->Find(findQuery[findCnt].first);
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
#ifdef EXTERNAL
        std::pair<KeyType, std::vector<KeyType>> data = {
            insertQuery[insertCnt].first,
            std::vector<KeyType>(1, insertQuery[i].second)};
        carmi->Insert(data);
#else
        carmi->Insert(insertQuery[insertCnt]);
#endif  // EXTERNAL
        insertCnt++;
      }
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  insertCnt = 0;
  s = std::chrono::system_clock::now();
  if (isZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) findCnt++;
      for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
#ifdef EXTERNAL
        std::pair<KeyType, std::vector<KeyType>> data = {
            insertQuery[insertCnt].first,
            std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
        insertCnt++;
      }
    }
  } else {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < length; j++) {
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insert_length; j++) {
#ifdef EXTERNAL
        std::pair<KeyType, std::vector<KeyType>> data = {
            insertQuery[insertCnt].first,
            std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
        insertCnt++;
      }
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

// read mostly workload (range scan)
// a mix of 95/5 reads and writes
template <typename CarmiType, typename KeyType, typename ValueType>
void WorkloadE(bool isZipfian, const carmi_params::TestDataVecType &findDataset,
               const carmi_params::TestDataVecType &insertDataset,
               const std::vector<int> &length, CarmiType *carmi) {
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
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = carmi->Find(findQuery[index[findCnt]].first);
        for (int l = 0; l < length[index[findCnt]] && it != it.end(); l++) {
          ret[l] = {it.key(), it.key()};
          ++it;
        }
        findCnt++;
      }
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
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
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
      carmi->Insert(data);
#else
      carmi->Insert(insertQuery[i]);
#endif  // EXTERNAL
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
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename CarmiType::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        typename CarmiType::iterator it;
        for (int l = 0; l < length[findCnt]; l++) {
        }
        findCnt++;
      }
#ifdef EXTERNAL
      std::pair<KeyType, std::vector<KeyType>> data = {
          insertQuery[i].first, std::vector<KeyType>(1, insertQuery[i].second)};
#endif  // EXTERNAL
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
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_
