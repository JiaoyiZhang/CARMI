/**
 * @file externalmap_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-22
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <map>

#include "../../include/carmi_external_map.h"
#include "gtest/gtest.h"

const int kTestMaxValue = 1000000;
const int kInitSize = 1000000;
const int kInsertSize = 100000;
unsigned int seed = time(NULL);

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;
typedef std::vector<DataType> DataVecType;
typedef std::map<KeyType, ValueType> MapType;

DataVecType initDataset(kInitSize);
DataVecType insertDataset(kInsertSize);
std::vector<KeyType> futureInsertKey(kInsertSize);
MapType map;
std::default_random_engine engine(time(0));
std::uniform_real_distribution<KeyType> dis(0, kTestMaxValue);
const int record_size = sizeof(KeyType) + sizeof(ValueType);

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

TEST(TestRangeConstructor, CARMIExternalMapRangeConstructor) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    initDataset[i] = {tmpKey, tmpKey * 10};
  }
  std::sort(initDataset.begin(), initDataset.end());
  KeyType lastKey = initDataset[kInitSize - 1].first;
  for (int i = 0; i < kInsertSize; i++) {
    lastKey += 1;
    insertDataset[i] = {lastKey, lastKey * 10};
    futureInsertKey[i] = insertDataset[i].first;
  }
  ASSERT_TRUE(carmi.empty());
  int extLen = kInitSize * 2 + kInsertSize * 2;
  externalDataset = new KeyType[extLen];
  for (int i = 0, j = 0; i < static_cast<int>(kInitSize); i++) {
    *(externalDataset + j) = initDataset[i].first;
    *(externalDataset + j + 1) = initDataset[i].second;
    j += 2;
  }
  CarmiType c(externalDataset, futureInsertKey, initDataset.size(),
              record_size);
  carmi.swap(c);
  MapType map_test(initDataset.begin(), initDataset.end());
  map.swap(map_test);

  ASSERT_FALSE(carmi.empty());
  ASSERT_FALSE(map.empty());

  ASSERT_EQ(carmi.size(), map.size());

  auto carmi_it = carmi.begin();
  auto map_it = map.begin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestRBegin, CARMIExternalMapRBegin) {
  CarmiType::reverse_iterator carmi_it = carmi.rbegin();
  auto map_it = map.rbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);
    carmi_it++;
    map_it++;
  }
}

TEST(TestCBegin, CARMIExternalMapCBegin) {
  auto carmi_it = carmi.cbegin();
  auto map_it = map.cbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestCRBegin, CARMIExternalMapCRBegin) {
  CarmiType::const_reverse_iterator carmi_it = carmi.crbegin();
  auto map_it = map.crbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestEnd, CARMIExternalMapEnd) {
  auto carmi_it = carmi.end();
  carmi_it--;
  auto map_it = map.find(initDataset[kInitSize - 1].first);

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first) << " i:" << i << std::endl;
    ASSERT_EQ(carmi_it.data(), map_it->second) << " i:" << i << std::endl;
    carmi_it--;
    map_it--;
  }
}

TEST(TestREnd, CARMIExternalMapREnd) {
  CarmiType::reverse_iterator carmi_it = carmi.rend();
  carmi_it--;
  auto map_it = map.find(initDataset[0].first);

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first) << " i:" << i << std::endl;
    ASSERT_EQ(carmi_it.data(), map_it->second) << " i:" << i << std::endl;
    carmi_it--;
    map_it++;
  }
}

TEST(TestCEnd, CARMIExternalMapCEnd) {
  CarmiType::const_iterator carmi_it = carmi.cend();
  carmi_it--;
  auto map_it = map.find(initDataset[kInitSize - 1].first);

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first) << " i:" << i << std::endl;
    ASSERT_EQ(carmi_it.data(), map_it->second) << " i:" << i << std::endl;

    carmi_it--;
    map_it--;
  }
}

TEST(TestCREnd, CARMIExternalMapCREnd) {
  CarmiType::const_reverse_iterator carmi_it = carmi.crend();
  carmi_it--;
  auto map_it = map.find(initDataset[0].first);

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first) << " i:" << i << std::endl;
    ASSERT_EQ(carmi_it.data(), map_it->second) << " i:" << i << std::endl;
    carmi_it--;
    map_it++;
  }
}

TEST(TestCARMIExternalMapFind, CARMIExternalMapFind) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_it = carmi.find(tmpKey);
    auto map_it = map.find(tmpKey);
    bool carmi_find = true, map_find = true;
    if (carmi_it == carmi.end()) {
      carmi_find = false;
    }
    if (map_it == map.end()) {
      map_find = false;
    }
    ASSERT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      ASSERT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCARMIExternalMapLowerbound, CARMIExternalMapLowerbound) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_it = carmi.lower_bound(tmpKey);
    auto map_it = map.lower_bound(tmpKey);
    bool carmi_find = true, map_find = true;
    if (carmi_it == carmi.end()) {
      carmi_find = false;
    }
    if (map_it == map.end()) {
      map_find = false;
    }
    ASSERT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      ASSERT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCARMIExternalMapUpperbound, CARMIExternalMapUpperbound) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_it = carmi.upper_bound(tmpKey);
    auto map_it = map.upper_bound(tmpKey);
    bool carmi_find = true, map_find = true;
    if (carmi_it == carmi.end()) {
      carmi_find = false;
    }
    if (map_it == map.end()) {
      map_find = false;
    }
    ASSERT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      ASSERT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCARMIExternalMapEqualRange, CARMIExternalMapEqualRange) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_it = carmi.equal_range(tmpKey).first;
    auto map_it = map.equal_range(tmpKey).first;
    bool carmi_find = true, map_find = true;
    if (carmi_it == carmi.end()) {
      carmi_find = false;
    }
    if (map_it == map.end()) {
      map_find = false;
    }
    ASSERT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      ASSERT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCARMIExternalMapCount, CARMIExternalMapCount) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_cnt = carmi.count(tmpKey);
    auto map_cnt = map.count(tmpKey);
    ASSERT_EQ(carmi_cnt, map_cnt);
  }
}

TEST(TestInsert, CARMIExternalMapInsert) {
  int cnt = 2 * kInitSize;
  for (int i = 0; i < kInsertSize; i++) {
    KeyType tmpKey = insertDataset[i].first;
    *(externalDataset + cnt) = insertDataset[i].first;
    *(externalDataset + cnt + 1) = insertDataset[i].second;
    cnt += 2;
    auto carmi_res = carmi.insert(tmpKey).first;
    auto map_res = map.insert({tmpKey, tmpKey * 10}).first;
    ASSERT_EQ(carmi_res.key(), map_res->first);

    auto carmi_it = carmi.begin();
  }
}

TEST(TestCARMIExternalMapClear, CARMIExternalMapClear) {
  carmi.clear();
  map.clear();

  ASSERT_TRUE(carmi.empty());
  ASSERT_TRUE(map.empty());

  ASSERT_EQ(carmi.size(), map.size());
}
