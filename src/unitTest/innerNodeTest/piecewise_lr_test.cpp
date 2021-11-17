/**
 * @file piecewise_lr_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../experiment/dataset/lognormal_distribution.h"
#include "../../include/nodes/innerNode/plr_model.h"
#include "gtest/gtest.h"

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;

std::vector<DataType> initData;
std::vector<DataType> insertData;
std::vector<DataType> testInsert;

const int kChildNum = 512;
const int kTestMaxValue = kMaxValue;

LognormalDataset logData(0.9);
PLRModel<double, double> model(kChildNum);

TEST(TestMultiTrain, MultiTrainPLRModel) {
  std::vector<DataType> testTrainData;
  unsigned int seed = time(NULL);
  for (int i = 0; i < 9; i++) {
    int tmpSize = std::pow(10, i) - 1;
    std::cout << "Start test size: " << tmpSize << std::endl;
    testTrainData = std::vector<DataType>(tmpSize);
    for (int j = 0; j < tmpSize; j++) {
      KeyType tmpKey = rand_r(&seed) % kTestMaxValue;
      testTrainData[j] = {tmpKey, tmpKey};
    }
    std::sort(testTrainData.begin(), testTrainData.end());
    std::cout << "Dataset is ready, start to test." << std::endl;
    PLRModel<KeyType, ValueType> tmpModel(kChildNum);
    tmpModel.Train(0, testTrainData.size(), testTrainData);
    EXPECT_EQ(kChildNum, tmpModel.flagNumber & 0x00FFFFFF);
    for (int j = 0; j < 5; j++) {
      EXPECT_LE(tmpModel.index[j], tmpModel.index[j + 1]);
    }
    for (int j = 0; j < 7; j++) {
      EXPECT_LT(tmpModel.keys[j], tmpModel.keys[j + 1]);
    }
    std::cout << "Subtest " << i << " over!" << std::endl;
  }
}

TEST(TestTrain, TrainPLRModel) {
  logData.GenerateDataset(&initData, &insertData, &testInsert);
  model.Train(0, initData.size(), initData);
  EXPECT_EQ(kChildNum, model.flagNumber & 0x00FFFFFF);
  EXPECT_EQ(2, model.flagNumber >> 24);
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