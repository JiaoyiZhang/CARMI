/**
 * @file cfarray_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../include/nodes/leafNode/cfarray_type.h"
#include "gtest/gtest.h"

const int kTestMaxValue = 10000;
unsigned int seed = time(NULL);
typedef double KeyType;
typedef double ValueType;
typedef CFArrayType<KeyType, ValueType> CFType;

TEST(TestCalNeededBlockNum, CalNeededBlockNum) {
  int CFSize = sizeof(CFType);
  ASSERT_EQ(64, CFSize);
  int maxBlockNum = CFType::kMaxBlockNum;
  for (int i = 0; i < CFType::kMaxLeafCapacity; i++) {
    int needBlockNum = CFType::CalNeededBlockNum(i);
    ASSERT_GE(needBlockNum, 0);
    ASSERT_LE(needBlockNum, maxBlockNum);
  }
}

TEST(TestSearchDataBlock, CheckSearchBlockRes) {
  for (int i = 0; i < CFType::kMaxBlockCapacity; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    LeafSlots<KeyType, ValueType> currblock;
    CFType tmpCFNode;
    for (int j = 0; j < i; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    for (int j = 0; j < i; j++) {
      currblock.slots[j] = testTrainData[j];
    }
    for (int j = 0; j < i; j++) {
      int res = tmpCFNode.SearchDataBlock(currblock, testTrainData[j].first, i);
      EXPECT_EQ(testTrainData[res].first, testTrainData[j].first)
          << "j:" << j << ",\tres:" << res << ",\ti:" << i << std::endl;
    }
  }
}

TEST(TestNormalStoreData, CheckStoreData) {
  int maxBlockNum = CFType::kMaxBlockNum;
  for (int i = 0; i < CFType::kMaxLeafCapacity; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    DataArrayStructure<KeyType, ValueType> data(maxBlockNum, i);
    CFType tmpCFNode;
    for (int t = 0; t < CFType::kMaxPerSizeNum; t++) {
      ASSERT_EQ(static_cast<int>(tmpCFNode.perSize[t]), 0);
    }
    for (int j = 0; j < i; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    int needBlockNum = CFType::CalNeededBlockNum(i);
    int tmpEnd = -1;
    auto isSuccess =
        tmpCFNode.StoreData(testTrainData, std::vector<int>(i), false,
                            needBlockNum, 0, &data, &tmpEnd);
    ASSERT_TRUE(isSuccess);
    for (int j = 0; j < CFType::kMaxBlockNum - 2; j++) {
      ASSERT_LE(tmpCFNode.slotkeys[j], tmpCFNode.slotkeys[j + 1]);
    }
    for (int t = 0; t < CFType::kMaxPerSizeNum; t++) {
      ASSERT_GE(static_cast<int>(tmpCFNode.perSize[t]), 0);
      ASSERT_LE(static_cast<int>(tmpCFNode.perSize[t]), 255);
    }
    for (int j = 0; j < i; j++) {
      for (int k = 0; k < CFType::kMaxBlockCapacity - 1; k++) {
        KeyType l = data.dataArray[j].slots[k].first;
        KeyType r = data.dataArray[j].slots[k + 1].first;
        ASSERT_LE(l, r) << "the size is:" << i;
      }
    }
  }
}

TEST(TestFind, CFArrayFindData) {
  int maxBlockNum = CFType::kMaxBlockNum;
  for (int i = 0; i < CFType::kMaxLeafCapacity; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    DataArrayStructure<KeyType, ValueType> data(maxBlockNum, i);
    CFType tmpCFNode;
    for (int j = 0; j < i; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    int needBlockNum = CFType::CalNeededBlockNum(i);
    int tmpEnd = -1;
    auto isSuccess =
        tmpCFNode.StoreData(testTrainData, std::vector<int>(i), false,
                            needBlockNum, 0, &data, &tmpEnd);
    for (int j = 0; j < i; j++) {
      int currblock = 0;
      int currslot = tmpCFNode.Find(data, testTrainData[j].first, &currblock);
      KeyType res =
          data.dataArray[tmpCFNode.m_left + currblock].slots[currslot].first;
      ASSERT_EQ(res, testTrainData[j].first)
          << "j:" << j << ",\tres:" << res << ",\ti:" << i;
    }
  }
}

TEST(TestInsert, InsertData) {
  int maxBlockNum = CFType::kMaxBlockNum;
  for (int i = 0; i < CFType::kMaxLeafCapacity; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    DataArrayStructure<KeyType, ValueType> data(maxBlockNum, i);
    CFType tmpCFNode;
    for (int j = 0; j < i; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    int needBlockNum = CFType::CalNeededBlockNum(i);
    int tmpEnd = -1;
    tmpCFNode.StoreData(testTrainData, std::vector<int>(i), false, needBlockNum,
                        0, &data, &tmpEnd);
    std::pair<KeyType, ValueType> datapoint = {rand_r(&seed) % kTestMaxValue,
                                               rand_r(&seed) % kTestMaxValue};
    int currblock = 0, currslot = 0;
    auto isSuccess = tmpCFNode.Insert(datapoint, &currblock, &currslot, &data);
    if (isSuccess) {
      int m_left = tmpCFNode.m_left;
      int blockNum = tmpCFNode.flagNumber & 0x00FFFFFF;
      int nowDataNum = CFType::GetDataNum(data, m_left, m_left + blockNum);

      ASSERT_EQ(i + 1, nowDataNum);
      for (int j = m_left; j < m_left + blockNum; j++) {
        for (int k = 0; k < CFType::kMaxBlockCapacity - 1; k++) {
          KeyType l = data.dataArray[j].slots[k].first;
          KeyType r = data.dataArray[j].slots[k + 1].first;
          ASSERT_LE(l, r);
        }
      }
    }
  }
}

TEST(TestDelete, DeleteData) {
  int maxBlockNum = CFType::kMaxBlockNum;
  int size = 90;
  std::vector<std::pair<KeyType, ValueType>> testTrainData(size);
  DataArrayStructure<KeyType, ValueType> data(maxBlockNum, size);
  CFType tmpCFNode;
  for (int j = 0; j < size; j++) {
    KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
    testTrainData[j] = {tmpKey, tmpKey * 10};
  }
  std::sort(testTrainData.begin(), testTrainData.end());
  int needBlockNum = CFType::CalNeededBlockNum(size);
  int tmpEnd = -1;
  tmpCFNode.StoreData(testTrainData, std::vector<int>(size), false,
                      needBlockNum, 0, &data, &tmpEnd);

  for (int j = 0; j < size; j += 5) {
    size_t cnt = 0;
    auto isSuccess = tmpCFNode.Delete(testTrainData[j].first, &cnt, &data);
    ASSERT_TRUE(isSuccess);
    ASSERT_GT(cnt, 0);
    int m_left = tmpCFNode.m_left;
    int blockNum = tmpCFNode.flagNumber & 0x00FFFFFF;
    int nowDataNum = CFType::GetDataNum(data, m_left, m_left + blockNum);
    for (int j = m_left; j < m_left + blockNum; j++) {
      for (int k = 0; k < CFType::kMaxBlockCapacity - 1; k++) {
        KeyType l = data.dataArray[j].slots[k].first;
        KeyType r = data.dataArray[j].slots[k + 1].first;
        ASSERT_LE(l, r);
      }
    }
  }
}