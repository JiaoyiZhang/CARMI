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
#include <vector>

#include "../../include/carmi.h"
#include "../../include/func/find_function.h"
#include "../../include/func/insert_function.h"
#include "./zipfian.h"

extern std::ofstream outRes;

void InitTestSet(int Ratio, const DataVectorType &findQueryset,
                 const DataVectorType &insertDataset, DataVectorType *findQuery,
                 DataVectorType *insertQuery, std::vector<int> *index) {
  (*findQuery) = findQueryset;  // -> findQueryset
  (*insertQuery) = insertDataset;

  std::default_random_engine engine;

  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  engine = std::default_random_engine(seed);
  shuffle((*findQuery).begin(), (*findQuery).end(), engine);

  if (!kPrimaryIndex) {
    unsigned seed1 =
        std::chrono::system_clock::now().time_since_epoch().count();
    engine = std::default_random_engine(seed1);
    shuffle((*insertQuery).begin(), (*insertQuery).end(), engine);
  }

  int end = round(kTestSize * Ratio);  // 50000
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, (*findQuery).size());
  *index = std::vector<int>(end, 0);
  for (int i = 0; i < end; i++) {
    int idx = zip.GenerateNextIndex();
    (*index)[i] = idx;
  }
}

void PrintAvgTime(double time) {
  std::cout << "total time:" << time * kSecondToNanosecond / kTestSize
            << std::endl;
  outRes << time * kSecondToNanosecond / kTestSize << ",";
}

// write heavy workload
// a mix of 50/50 reads and writes
void WorkloadA(const DataVectorType &findDataset,
               const DataVectorType &insertDataset, CARMI *carmi) {
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kWriteHeavy;
  InitTestSet(kWriteHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
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
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
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
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

// read heavy workload
// a mix of 95/5 reads and writes
void WorkloadB(const DataVectorType &findDataset,
               const DataVectorType &insertDataset, CARMI *carmi) {
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> index;
  InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) {
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
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19; j++) findCnt++;
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) findCnt++;
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
void WorkloadC(const DataVectorType &findDataset, CARMI *carmi) {
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> index;
  int end = kTestSize * kReadOnly;
  InitTestSet(kReadOnly, findDataset, DataVectorType(), &findQuery,
              &insertQuery, &index);

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
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
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;
  tmp -= tmp0;

  PrintAvgTime(tmp);
}

// write partial workload
// a mix of 85/15 reads and writes
void WorkloadD(const DataVectorType &findDataset,
               const DataVectorType &insertDataset, CARMI *carmi) {
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> index;
  InitTestSet(kWritePartial, findDataset, insertDataset, &findQuery,
              &insertQuery, &index);

  int length = round(kTestSize * kWritePartial);
  int insert_length = round(kTestSize * (1 - kWritePartial));

  int findCnt = 0, insertCnt = 0;

  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) {
        carmi->Find(findQuery[index[findCnt]].first);
        findCnt++;
      }
      for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) {
        carmi->Insert(insertQuery[insertCnt]);
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
        carmi->Insert(insertQuery[insertCnt]);
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
  if (kZipfian) {
    for (int i = 0; i < insert_length; i++) {
      for (int j = 0; j < 17 && findCnt < findQuery.size(); j++) findCnt++;
      for (int j = 0; j < 3 && insertCnt < insertQuery.size(); j++) insertCnt++;
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
void WorkloadE(const DataVectorType &findDataset,
               const DataVectorType &insertDataset,
               const std::vector<int> &length, CARMI *carmi) {
  DataVectorType findQuery;
  DataVectorType insertQuery;
  std::vector<int> index;
  InitTestSet(kReadHeavy, findDataset, insertDataset, &findQuery, &insertQuery,
              &index);

  int end = round(kTestSize * (1 - kReadHeavy));
  int findCnt = 0;

  DataVectorType ret(100, {-1, -1});
  std::chrono::_V2::system_clock::time_point s, e;
  double tmp;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        auto it = carmi->Find(findQuery[index[findCnt]].first);
        for (int l = 0; l < length[index[findCnt]] && it != it.end(); l++) {
          ret[l] = {it.key(), it.data()};
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
        for (int l = 0; l < length[findCnt] && it != it.end(); l++) {
          ret[l] = {it.key(), it.data()};
          ++it;
        }
        findCnt++;
      }
      carmi->Insert(insertQuery[i]);
    }
  }
  e = std::chrono::system_clock::now();
  tmp =
      static_cast<double>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(e - s).count()) /
      std::chrono::nanoseconds::period::den;

  findCnt = 0;
  s = std::chrono::system_clock::now();
  if (kZipfian) {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        CARMI::iterator it;
        for (int l = 0; l < length[index[findCnt]]; l++) {
        }
        findCnt++;
      }
    }
  } else {
    for (int i = 0; i < end; i++) {
      for (int j = 0; j < 19 && findCnt < findQuery.size(); j++) {
        CARMI::iterator it;
        for (int l = 0; l < length[findCnt]; l++) {
        }
        findCnt++;
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
#endif  // SRC_EXPERIMENT_WORKLOAD_WORKLOADS_H_
