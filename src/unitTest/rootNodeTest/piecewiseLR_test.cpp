/**
 * @file piecewiseLR_test.cpp
 * @author Jiaoyi
 * @brief
 * @version 0.1
 * @date 2021-11-03
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "../../include/nodes/rootNode/trainModel/piecewiseLR.h"

#include "../../experiment/dataset/lognormal_distribution.h"
#include "gtest/gtest.h"

std::vector<std::pair<double, double>> initData;
std::vector<std::pair<double, double>> insertData;
std::vector<std::pair<double, double>> testInsert;

const int kChildNum = 512;
const int kTestMaxValue = kMaxValue;

LognormalDataset logData(0.9);
PiecewiseLR<DataVecType, double> model;

TEST(TestTrain, TrainPLRModel) {
  logData.GenerateDataset(&initData, &insertData, &testInsert);
  model.maxChildIdx = kChildNum - 1;
  model.Train(initData);
  EXPECT_EQ(kChildNum - 1, model.maxChildIdx);
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