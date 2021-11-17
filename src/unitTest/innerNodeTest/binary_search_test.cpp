/**
 * @file binary_search_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../experiment/dataset/lognormal_distribution.h"
#include "../../include/nodes/innerNode/bs_model.h"
#include "gtest/gtest.h"

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;

std::vector<DataType> initData;
std::vector<DataType> insertData;
std::vector<DataType> testInsert;

const int kChildNum = 15;
const int kTestMaxValue = kMaxValue;

LognormalDataset logData(0.9);
BSModel<KeyType, ValueType> model(kChildNum);

TEST(TestMultiTrain, MultiTrainBSModel) {
  std::vector<DataType> testTrainData;
  unsigned int seed = time(NULL);
  for (int i = 0; i < 9; i++) {
    int tmpSize = std::pow(10, i) - 1;
    testTrainData = std::vector<DataType>(tmpSize);
    for (int j = 0; j < tmpSize; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    BSModel<KeyType, ValueType> tmpModel(kChildNum);
    tmpModel.Train(0, testTrainData.size(), testTrainData);
    EXPECT_EQ(kChildNum, tmpModel.flagNumber & 0x00FFFFFF); 
    for (int j = 0; j < 13; j++) {
      EXPECT_LE(tmpModel.keys[j], tmpModel.keys[j + 1]);
    }
  }
}

TEST(TestTrain, TrainBSModel) {
  logData.GenerateDataset(&initData, &insertData, &testInsert);
  model.Train(0, initData.size(), initData);
  EXPECT_EQ(kChildNum, model.flagNumber & 0x00FFFFFF);
  EXPECT_EQ(4, model.flagNumber >> 24);
}

TEST(TestPredictInitData, PredictInitData) {
  for (int i = 0; i < initData.size(); i++) {
    int p = model.Predict(initData[i].first);
    EXPECT_GE(p, 0);
    EXPECT_LT(p, kChildNum);
  }
}

TEST(TestPredictInsertData, PredictInsertData) {
  for (int i = 0; i < insertData.size(); i++) {
    int p = model.Predict(insertData[i].first);
    EXPECT_GE(p, 0);
    EXPECT_LT(p, kChildNum);
  }
}