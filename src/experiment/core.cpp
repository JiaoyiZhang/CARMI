/**
 * @file core.cpp
 * @author Jiaoyi
 * @brief
 * @version 3.0
 * @date 2021-03-16
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <vector>

#include "../include/construct/construction.h"
#include "../include/func/calculate_space.h"
#include "../include/func/get_node_info.h"
#include "./experiment_params.h"
#include "./functions.h"

extern std::ofstream outRes;

/**
 * @brief the function of using CARMI
 *
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] initRatio the workload type
 * @param[in] rate the weight of space
 * @param[in] length the length of range scan
 * @param[in] initDataset
 * @param[in] testInsertQuery
 */
void CoreCARMI(bool isZipfian, double initRatio, double rate,
               const std::vector<int> &length, const DataVecType &initDataset,
               const DataVecType &insertDataset,
               const DataVecType &testInsertQuery) {
#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "kRate: " << rate << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  typedef CARMIMap<KeyType, ValueType> CarmiType;
  CarmiType carmi(initDataset.begin(), initDataset.end(), insertDataset.begin(),
                  insertDataset.end(), rate);

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << std::endl;

  std::cout << "\nprint the space:" << std::endl;
  auto space = carmi.CalculateSpace() / 1024.0 / 1024.0;
  outRes << space << ",";
  std::cout << space << " MB\n";

#endif

  if (initRatio == kWriteHeavy)
    WorkloadA<KeyType, ValueType>(isZipfian, initDataset, testInsertQuery,
                                  &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB<KeyType, ValueType>(isZipfian, initDataset, testInsertQuery,
                                  &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC<KeyType, ValueType>(isZipfian, initDataset,
                                  &carmi);  // read-only
  else if (initRatio == kWritePartial)
    WorkloadD<KeyType, ValueType>(isZipfian, initDataset, testInsertQuery,
                                  &carmi);  // write-partial
  else if (initRatio == kRangeScan)
    WorkloadE<KeyType, ValueType>(isZipfian, initDataset, testInsertQuery,
                                  length,
                                  &carmi);  // range scan
}

template <typename KeyType, typename ValueType>
class ExternalDataType {
 public:
  typedef ValueType ValueType_;
  ExternalDataType() {
    k = 0;
    v = 0;
  }
  explicit ExternalDataType(KeyType key, ValueType_ value) {
    k = key;
    v = value;
  }
  const KeyType &key() const { return k; }
  const ValueType_ &data() const { return v; }

  bool operator<(const ExternalDataType &a) const {
    if (k == a.k) {
      return v < a.v;
    }
    return k < a.k;
  }

  KeyType k;
  ValueType_ v;
};

/**
 * @brief the function of using external CARMI
 *
 * @param[in] isZipfian whether to use zipfian access during the test
 * @param[in] initRatio the workload type
 * @param[in] rate the weight of space
 * @param[in] length the length of range scan
 * @param[in] initDataset
 * @param[in] testInsertQuery
 */
void CoreExternalCARMI(bool isZipfian, double initRatio, double rate,
                       const std::vector<int> &length,
                       const DataVecType &initDataset,
                       const DataVecType &testInsertQuery) {
  DataVecType init = initDataset;

#ifdef DEBUG
  std::cout << std::endl;
  std::cout << "-------------------------------" << std::endl;
  std::cout << "Start construction!" << std::endl;
  time_t timep;
  time(&timep);
  char tmpTime[64];
  strftime(tmpTime, sizeof(tmpTime), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "\nTEST time: " << tmpTime << std::endl;
#endif

  KeyType *externalDataset;
  const int record_size = sizeof(KeyType) + sizeof(ValueType);
  typedef CARMIExternalMap<KeyType, ExternalDataType<KeyType, ValueType>>
      CarmiType;
  int extLen = initDataset.size() * 2 + kTestSize * 2;
  externalDataset = new KeyType[extLen];
  for (int i = 0, j = 0; i < static_cast<int>(initDataset.size()); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }
  std::vector<KeyType> futureInsertKey(testInsertQuery.size(), 0);
  for (int i = 0; i < static_cast<int>(testInsertQuery.size()); i++) {
    futureInsertKey[i] = testInsertQuery[i].first;
  }
  // initDataset -> only includes the findQuery
  CarmiType carmi(externalDataset, futureInsertKey, initDataset.size(),
                  record_size, rate);

#ifdef DEBUG
  time(&timep);
  char tmpTime1[64];
  strftime(tmpTime1, sizeof(tmpTime1), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  std::cout << "finish time: " << tmpTime1 << std::endl;
#endif

  if (initRatio == kWriteHeavy)
    WorkloadA<KeyType, ExternalDataType<KeyType, ValueType>>(
        isZipfian, init, testInsertQuery,
        &carmi);  // write-heavy
  else if (initRatio == kReadHeavy)
    WorkloadB<KeyType, ExternalDataType<KeyType, ValueType>>(
        isZipfian, init, testInsertQuery,
        &carmi);  // read-heavy
  else if (initRatio == kReadOnly)
    WorkloadC<KeyType, ExternalDataType<KeyType, ValueType>>(
        isZipfian, init,
        &carmi);  // read-only
  else if (initRatio == kRangeScan)
    WorkloadE<KeyType, ExternalDataType<KeyType, ValueType>>(
        isZipfian, init, testInsertQuery, length,
        &carmi);  // range scan
}
