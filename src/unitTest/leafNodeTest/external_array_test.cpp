
/**
 * @file external_array_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-04
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../include/nodes/leafNode/external_array_type.h"
#include "gtest/gtest.h"

typedef double KeyType;
typedef double ValueType;

const int kTestMaxValue = 10000;
unsigned int seed = time(NULL);
template <typename KeyType, typename ValueType>
class DataType {
 public:
  typedef ValueType ValueType_;
  DataType() {
    k = 0;
    v = 0;
  }
  explicit DataType(KeyType key, ValueType_ value) {
    k = key;
    v = value;
  }
  const KeyType& key() const { return k; }
  const ValueType_& data() const { return v; }

  bool operator<(const DataType& a) const {
    if (k == a.k) {
      return v < a.v;
    }
    return k < a.k;
  }

  KeyType k;
  ValueType_ v;
};

TEST(TestTrain, TrainExternalArrayNode) {
  for (int i = 0; i < carmi_params::kMaxLeafNodeSizeExternal; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    ExternalArray<KeyType, ValueType> externalNode;
    for (int j = 0; j < i; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    externalNode.Train(testTrainData, 0, i);
    EXPECT_GE(externalNode.error, 0);
  }
}

TEST(TestFind, ExternalArrayNodeFind) {
  for (int i = 0; i < carmi_params::kMaxLeafNodeSizeExternal; i++) {
    std::vector<std::pair<KeyType, ValueType>> testTrainData(i);
    ExternalArray<KeyType, ValueType> externalNode;
    KeyType* externalDataset = new KeyType[i * 2];
    for (int j = 0, k = 0; j < i; j++, k += 2) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey * 10};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    for (int j = 0, k = 0; j < i; j++, k += 2) {
      *(externalDataset + k) = testTrainData[j].first;
      *(externalDataset + k + 1) = testTrainData[j].second;
    }
    externalNode.m_left = 0;
    externalNode.Train(testTrainData, 0, i);
    for (int j = 0; j < i; j++) {
      int currslot =
          externalNode.Find(testTrainData[j].first, 16, externalDataset);
      KeyType res = testTrainData[currslot].first;
      ASSERT_EQ(res, testTrainData[j].first);
    }
  }
}