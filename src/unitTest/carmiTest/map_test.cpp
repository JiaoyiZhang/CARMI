/**
 * @file map_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-22
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <map>

#include "../../include/carmi_map.h"
#include "gtest/gtest.h"

const int kTestMaxValue = 1000000;
const int kInitSize = 1000000;
const int kInsertSize = 100000;
unsigned int seed = time(NULL);

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;
typedef std::vector<DataType> DataVecType;
typedef CARMIMap<KeyType, ValueType> CarmiType;
typedef std::map<KeyType, ValueType> MapType;

DataVecType initDataset(kInitSize);
DataVecType insertDataset(kInsertSize);
DataVecType testInsertQuery(kInsertSize);
CarmiType carmi;
MapType map;
std::default_random_engine engine(time(0));
std::uniform_real_distribution<KeyType> dis(0, kTestMaxValue);

TEST(TestDefaultConstructor, CARMIMapDefaultConstructor) {
  CarmiType carmi_test;
  MapType map_test;
  ASSERT_TRUE(carmi_test.empty());
  ASSERT_TRUE(map_test.empty());

  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    carmi_test.insert({tmpKey, tmpKey * 10});
    map_test.insert({tmpKey, tmpKey * 10});
  }
  ASSERT_EQ(carmi_test.size(), map_test.size());

  auto carmi_it = carmi_test.begin();
  auto map_it = map_test.begin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestOperatorConstructor, CARMIMapOperatorConstructor) {
  CarmiType carmi_test;
  MapType map_test;
  ASSERT_TRUE(carmi_test.empty());
  ASSERT_TRUE(map_test.empty());

  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    map_test[tmpKey] = tmpKey * 10;
    carmi_test[tmpKey] = tmpKey * 10;
  }
  ASSERT_EQ(carmi_test.size(), map_test.size());

  auto carmi_it = carmi_test.begin();
  auto map_it = map_test.begin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestRangeConstructor, CARMIMapRangeConstructor) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    initDataset[i] = {tmpKey, tmpKey * 10};
  }
  std::sort(initDataset.begin(), initDataset.end());
  CarmiType carmi_test(initDataset.begin(), initDataset.end());
  carmi.swap(carmi_test);
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

TEST(TestRBegin, CARMIMapRBegin) {
  CarmiType::reverse_iterator carmi_it = carmi.rbegin();
  auto map_it = map.rbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);
    carmi_it++;
    map_it++;
  }
}

TEST(TestCBegin, CARMIMapCBegin) {
  auto carmi_it = carmi.cbegin();
  auto map_it = map.cbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestCRBegin, CARMIMapCRBegin) {
  CarmiType::const_reverse_iterator carmi_it = carmi.crbegin();
  auto map_it = map.crbegin();

  for (int i = 0; i < kInitSize; i++) {
    ASSERT_EQ(carmi_it.key(), map_it->first);
    ASSERT_EQ(carmi_it.data(), map_it->second);

    carmi_it++;
    map_it++;
  }
}

TEST(TestEnd, CARMIMapEnd) {
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

TEST(TestREnd, CARMIMapREnd) {
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

TEST(TestCEnd, CARMIMapCEnd) {
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

TEST(TestCREnd, CARMIMapCREnd) {
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

TEST(TestCarmimapFind, CARMIMapFind) {
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
    EXPECT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      EXPECT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCarmimapLowerbound, CARMIMapLowerbound) {
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
    EXPECT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      EXPECT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCarmimapUpperbound, CARMIMapUpperbound) {
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
    EXPECT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      EXPECT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCarmimapEqualRange, CARMIMapEqualRange) {
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
    EXPECT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      EXPECT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCarmimapCount, CARMIMapCount) {
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_cnt = carmi.count(tmpKey);
    auto map_cnt = map.count(tmpKey);
    EXPECT_EQ(carmi_cnt, map_cnt);
  }
}

TEST(TestInsert, CARMIMapInsert) {
  for (int i = 0; i < kInsertSize; i++) {
    KeyType tmpKey = dis(engine);
    auto carmi_res = carmi.insert({tmpKey, tmpKey * 10}).first;
    auto map_res = map.insert({tmpKey, tmpKey * 10}).first;
    EXPECT_EQ(carmi_res.key(), map_res->first);
  }
}

TEST(TestCarmimapErase, CARMIMapErase) {
  for (int i = 0; i < kInsertSize; i++) {
    KeyType tmpKey = dis(engine);
    auto old_it = carmi.find(tmpKey);
    int carmi_cnt = carmi.erase(tmpKey);
    int map_cnt = map.erase(tmpKey);
    EXPECT_EQ(carmi_cnt, map_cnt);

    auto carmi_it = carmi.upper_bound(tmpKey);
    auto map_it = map.upper_bound(tmpKey);
    bool carmi_find = true, map_find = true;
    if (carmi_it == carmi.end()) {
      carmi_find = false;
    }
    if (map_it == map.end()) {
      map_find = false;
    }
    EXPECT_EQ(carmi_find, map_find);
    if (carmi_find && map_find) {
      EXPECT_EQ(carmi_it.key(), map_it->first);
    }
  }
}

TEST(TestCarmimapClear, CARMIMapClear) {
  carmi.clear();
  map.clear();

  ASSERT_TRUE(carmi.empty());
  ASSERT_TRUE(map.empty());

  EXPECT_EQ(carmi.size(), map.size());
}
