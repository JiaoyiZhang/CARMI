/**
 * @file histogram_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "../../experiment/dataset/lognormal_distribution.h"
#include "../../include/nodes/innerNode/his_model.h"
#include "gtest/gtest.h"

typedef double KeyType;
typedef double ValueType;
typedef std::pair<KeyType, ValueType> DataType;

std::vector<DataType> initData;
std::vector<DataType> insertData;
std::vector<DataType> testInsert;

const int kChildNum = 256;
const int kTestMaxValue = kMaxValue;

LognormalDataset logData(0.9);
HisModel<double, double> model(kChildNum);

TEST(TestMultiTrain, MultiTrainHisModel) {
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
    HisModel<KeyType, ValueType> tmpModel(kChildNum);
    tmpModel.Train(0, testTrainData.size(), testTrainData);
    EXPECT_EQ(kChildNum, tmpModel.flagNumber & 0x00FFFFFF);
    EXPECT_EQ(3, tmpModel.flagNumber >> 24);
    EXPECT_NE(0, tmpModel.divisor);
    for (int j = 0; j < 16; j++) {
      EXPECT_GE(tmpModel.base[j], 0);
      EXPECT_LT(tmpModel.base[j], kChildNum);
    }
    for (int j = 0; j < 255; j++) {
      int l = tmpModel.offset[(j >> 4)] >> (15 - (j & 0x0000000F));
      l = (l & 0x55555555) + ((l >> 1) & 0x55555555);
      l = (l & 0x33333333) + ((l >> 2) & 0x33333333);
      l = (l & 0x0f0f0f0f) + ((l >> 4) & 0x0f0f0f0f);
      l = (l & 0x00ff00ff) + ((l >> 8) & 0x00ff00ff);
      l += tmpModel.base[(j >> 4)];

      int r = tmpModel.offset[((j + 1) >> 4)] >> (15 - ((j + 1) & 0x0000000F));
      r = (r & 0x55555555) + ((r >> 1) & 0x55555555);
      r = (r & 0x33333333) + ((r >> 2) & 0x33333333);
      r = (r & 0x0f0f0f0f) + ((r >> 4) & 0x0f0f0f0f);
      r = (r & 0x00ff00ff) + ((r >> 8) & 0x00ff00ff);
      r += tmpModel.base[((j + 1) >> 4)];
      EXPECT_LE(l, r);
    }
    std::cout << "Subtest " << i << " over!" << std::endl;
  }
}

TEST(TestTrain, TrainHisModel) {
  logData.GenerateDataset(&initData, &insertData, &testInsert);
  model.Train(0, initData.size(), initData);
  EXPECT_EQ(kChildNum, model.flagNumber & 0x00FFFFFF);
  EXPECT_EQ(3, model.flagNumber >> 24);
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