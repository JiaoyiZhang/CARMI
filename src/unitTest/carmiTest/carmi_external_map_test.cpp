/**
 * @file carmi_external_map_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../include/carmi_external_map.h"

#include "gtest/gtest.h"

const int kTestMaxValue = 1000000;
const int kInitSize = 10000;
const int kInsertSize = 100;
const float kRate = 0.1;
unsigned int seed = time(NULL);

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;
typedef std::vector<DataType> DataVecType;

const int record_size = sizeof(KeyType) + sizeof(ValueType);

DataVecType initDataset(kInitSize);
DataVecType insertDataset(kInsertSize);

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

typedef CARMIExternalMap<KeyType, ExternalDataType<KeyType, ValueType>>
    CarmiType;
CarmiType carmi;
KeyType *externalDataset;

TEST(TestCarmiExtmapConstructor, CARMIExtMapConstructor) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
    initDataset[i] = {tmpKey, tmpKey * 10};
  }
  std::sort(initDataset.begin(), initDataset.end());
  KeyType lastKey = initDataset[kInitSize - 1].first;
  std::vector<KeyType> futureInsertKey(kInsertSize);
  for (int i = 0; i < kInsertSize; i++) {
    lastKey += 1;
    insertDataset[i] = {lastKey, lastKey * 10};
    futureInsertKey[i] = insertDataset[i].first;
  }
  std::sort(insertDataset.begin(), insertDataset.end());
  ASSERT_TRUE(carmi.empty());
  int extLen = initDataset.size() * 2 + kInsertSize * 2;
  externalDataset = new KeyType[extLen];
  for (int i = 0, j = 0; i < static_cast<int>(initDataset.size()); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;  // due to <double, double>
  }

  CarmiType c(externalDataset, futureInsertKey, initDataset.size(), record_size,
              kRate);

  carmi.swap(c);

  ASSERT_EQ(carmi.size(), kInitSize);
  ASSERT_FALSE(carmi.empty());

  auto it = carmi.begin();
  for (int i = 0; i < kInitSize; i++) {
    EXPECT_EQ(it.key(), initDataset[i].first) << " i:" << i << std::endl;
    EXPECT_EQ(it.data(), initDataset[i].second);
    it++;
  }
}

TEST(TestCarmiExtmapFind, CARMIExtMapFind) {
  for (int i = 0; i < kInitSize; i++) {
    auto it = carmi.find(initDataset[i].first);
    EXPECT_EQ(it.key(), initDataset[i].first);
    EXPECT_EQ(it.data(), initDataset[i].second);
  }
}

TEST(TestCarmiExtmapLowerbound, CARMIExtMapLowerbound) {
  for (int i = 0; i < kInitSize; i++) {
    auto it = carmi.lower_bound(initDataset[i].first);
    EXPECT_EQ(it.key(), initDataset[i].first);
  }
  for (int i = 0; i < kInsertSize; i++) {
    if (insertDataset[i].first < initDataset[kInitSize - 1].first) {
      auto it = carmi.lower_bound(insertDataset[i].first);
      EXPECT_GE(it.key(), insertDataset[i].first);
    }
  }
}

TEST(TestCarmiExtmapUpperbound, CARMIExtMapUpperbound) {
  for (int i = 0; i < kInitSize - 1; i++) {
    auto it = carmi.upper_bound(initDataset[i].first);
    EXPECT_GT(it.key(), initDataset[i].first);
  }
}

TEST(TestCarmiExtmapEqualRange, CARMIExtMapEqualRange) {
  for (int i = 0; i < kInitSize; i++) {
    auto res = carmi.equal_range(initDataset[i].first);
    for (auto it = res.first; it != res.second; it++) {
      EXPECT_EQ(it.key(), initDataset[i].first);
    }
  }
}

TEST(TestCarmiExtmapCount, CARMIExtMapCount) {
  for (int i = 0; i < kInitSize; i++) {
    auto res = carmi.count(initDataset[i].first);
    auto vector_res =
        std::count(initDataset.begin(), initDataset.end(), initDataset[i]);
    EXPECT_EQ(res, vector_res);
  }
}

TEST(TestCarmiExtmapInsert, CARMIExtMapInsert) {
  int cnt = 2 * kInitSize;
  for (int i = 0; i < kInsertSize; i++, cnt += 2) {
    auto it = carmi.insert(insertDataset[i].first);
    *(externalDataset + cnt) = insertDataset[i].first;
    *(externalDataset + cnt + 1) = insertDataset[i].second;
    EXPECT_TRUE(it.second);
    EXPECT_EQ(it.first.key(), insertDataset[i].first);
    EXPECT_EQ(it.first.data(), insertDataset[i].second);
    for (int j = 0; j < i; j++) {
      auto res = carmi.find(insertDataset[j].first);
      EXPECT_EQ(res.key(), insertDataset[j].first);
    }
    for (int j = 0; j < kInitSize; j++) {
      auto res = carmi.find(initDataset[j].first);
      EXPECT_EQ(res.key(), initDataset[j].first);
      EXPECT_EQ(res.data(), initDataset[j].second);
    }
  }
}
