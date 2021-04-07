
/**
 * @file public_functions.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-04-07
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "public_functions.h"

#include "../../params.h"

/**
 * @brief prepare query workloads
 *
 * @param Ratio the ratio of find queries
 * @param findQueryset
 * @param insertDataset
 * @param findQuery
 * @param insertQuery
 * @param index
 */
void InitTestSet(double Ratio,
                 const carmi_params::TestDataVecType &findQueryset,
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

  int end = round(carmi_params::kTestSize * Ratio);
  Zipfian zip;
  zip.InitZipfian(PARAM_ZIPFIAN, (*findQuery).size());
  *index = std::vector<int>(end, 0);
  for (int i = 0; i < end; i++) {
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
  std::cout << "total time:"
            << time * carmi_params::kSecondToNanosecond /
                   carmi_params::kTestSize
            << std::endl;
  outRes << time * carmi_params::kSecondToNanosecond / carmi_params::kTestSize
         << ",";
}