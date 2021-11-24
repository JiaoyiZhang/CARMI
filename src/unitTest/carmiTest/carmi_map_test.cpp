/**
 * @file carmi_map_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-14
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../include/carmi_map.h"

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
typedef CARMIMap<KeyType, ValueType> CarmiType;

DataVecType initDataset(kInitSize);
DataVecType insertDataset(kInsertSize);
DataVecType testInsertQuery(kInsertSize);
CarmiType carmi;
std::default_random_engine engine(time(0));

TEST(TestCarmimapConstructor, CARMIMapConstructor) {
  std::uniform_real_distribution<KeyType> dis(0, kTestMaxValue);
  for (int i = 0; i < kInitSize; i++) {
    KeyType tmpKey = dis(engine);
    initDataset[i] = {tmpKey, tmpKey * 10};
  }
  std::sort(initDataset.begin(), initDataset.end());
  for (int i = 0; i < kInsertSize; i++) {
    KeyType tmpKey = dis(engine);
    insertDataset[i] = {tmpKey, tmpKey * 10};
  }
  std::sort(insertDataset.begin(), insertDataset.end());
  for (int i = 0; i < kInsertSize; i++) {
    KeyType tmpKey = dis(engine);
    testInsertQuery[i] = {tmpKey, tmpKey * 10};
  }
  ASSERT_TRUE(carmi.empty());

  CarmiType c(initDataset.begin(), initDataset.end(), insertDataset.begin(),
              insertDataset.end(), kRate);
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

TEST(TestCarmimapFind, CARMIMapFind) {
  for (int i = 0; i < kInitSize; i++) {
    auto it = carmi.find(initDataset[i].first);
    EXPECT_EQ(it.key(), initDataset[i].first);
    EXPECT_EQ(it.data(), initDataset[i].second);
  }
}

TEST(TestCarmimapLowerbound, CARMIMapLowerbound) {
  for (int i = 0; i < kInitSize; i++) {
    auto it = carmi.lower_bound(initDataset[i].first);
    EXPECT_EQ(it.key(), initDataset[i].first);
  }
  for (int i = 0; i < kInsertSize; i++) {
    if (testInsertQuery[i].first < initDataset[kInitSize - 1].first) {
      auto it = carmi.lower_bound(testInsertQuery[i].first);
      auto vector_res = std::lower_bound(initDataset.begin(), initDataset.end(),
                                         testInsertQuery[i]) -
                        initDataset.begin();
      EXPECT_EQ(it.key(), initDataset[vector_res].first);
    }
  }
}

TEST(TestCarmimapUpperbound, CARMIMapUpperbound) {
  for (int i = 0; i < kInitSize - 1; i++) {
    auto it = carmi.upper_bound(initDataset[i].first);
    EXPECT_GT(it.key(), initDataset[i].first);
  }
}

TEST(TestCarmimapEqualRange, CARMIMapEqualRange) {
  for (int i = 0; i < kInitSize; i++) {
    auto res = carmi.equal_range(initDataset[i].first);
    for (auto it = res.first; it != res.second; it++) {
      EXPECT_EQ(it.key(), initDataset[i].first);
    }
  }
}

TEST(TestCarmimapCount, CARMIMapCount) {
  for (int i = 0; i < kInitSize; i++) {
    auto res = carmi.count(initDataset[i].first);
    int cnt = 0;
    auto vector_res =
        std::count(initDataset.begin(), initDataset.end(), initDataset[i]);
    EXPECT_EQ(res, vector_res);
  }
}

TEST(TestCarmimapInsert, CARMIMapInsert) {
  for (int i = 0; i < kInsertSize; i++) {
    auto it = carmi.insert(testInsertQuery[i]);
    EXPECT_TRUE(it.second);
    EXPECT_EQ(it.first.key(), testInsertQuery[i].first);
    EXPECT_EQ(it.first.data(), testInsertQuery[i].second);
    for (int j = 0; j < i; j++) {
      auto res = carmi.find(testInsertQuery[j].first);
      EXPECT_EQ(res.key(), testInsertQuery[j].first);
    }
    for (int j = 0; j < kInitSize; j++) {
      auto res = carmi.find(initDataset[j].first);
      EXPECT_EQ(res.key(), initDataset[j].first);
      EXPECT_EQ(res.data(), initDataset[j].second);
    }
  }
}

TEST(TestCarmimapErase, CARMIMapErase) {
  for (int i = 0; i < kInsertSize; i++) {
    carmi.erase(testInsertQuery[i].first);
    auto it = carmi.find(testInsertQuery[i].first);
    EXPECT_EQ(it, carmi.end());
  }
}
