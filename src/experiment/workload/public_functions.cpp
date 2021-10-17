
/**
 * @file public_functions.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "public_functions.h"

#include <ctime>

#include "../experiment_params.h"

/**
 * @brief prepare query workloads
 *
 * @param findQueryset
 * @param insertDataset
 * @param findQuery
 * @param insertQuery
 * @param index
 */
void InitTestSet(const DataVecType &findQueryset,
                 const DataVecType &insertDataset, DataVecType *findQuery,
                 DataVecType *insertQuery, std::vector<int> *index) {
  (*findQuery) = findQueryset;
  (*insertQuery) = insertDataset;

  std::default_random_engine engine;

  unsigned seed = std::clock();
  engine = std::default_random_engine(seed);
  shuffle((*findQuery).begin(), (*findQuery).end(), engine);

  if (!kPrimaryIndex) {
    unsigned seed1 = std::clock();
    engine = std::default_random_engine(seed1);
    shuffle((*insertQuery).begin(), (*insertQuery).end(), engine);
  }

  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, (*findQuery).size());
  *index = std::vector<int>(kTestSize, 0);
  for (int i = 0; i < kTestSize; i++) {
    int idx = zip.GenerateNextIndex();
    (*index)[i] = idx;
  }
}

/**
 * @brief print the average time of the workload
 *
 * @param time
 */
void PrintAvgTime(double time) {
  std::cout << "average time:" << time * kSecondToNanosecond / kTestSize
            << std::endl;
  outRes << time * kSecondToNanosecond / kTestSize << ",";
}