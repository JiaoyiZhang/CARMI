// /**
//  * @file fixed_experiment.h
//  * @author Jiaoyi
//  * @brief
//  * @version 0.1
//  * @date 2021-03-16
//  *
//  * @copyright Copyright (c) 2021
//  *
//  */
// #ifndef SRC_STATICRMI_FIXED_EXPERIMENT_H_
// #define SRC_STATICRMI_FIXED_EXPERIMENT_H_
// #include <algorithm>
// #include <vector>

// #include "../experiment/dataset/exponential_distribution.h"
// #include "../experiment/dataset/lognormal_distribution.h"
// #include "../experiment/dataset/longitudes.h"
// #include "../experiment/dataset/normal_distribution.h"
// #include "../experiment/dataset/uniform_distribution.h"
// #include "./static.h"

// void fixedSynthetic(double initRatio, int kLeafID,
//                     const std::vector<int> &lengt);

// void fixedExperiment() {
//   // for range scan
//   std::vector<int> length;
//   // static structure
//   for (int i = 0; i < 2; i++) {
//     std::cout << "kleafnode:" << i << std::endl;
//     fixedSynthetic(kReadOnly, i, length);
//     if (i == 1) fixedSynthetic(kWriteHeavy, i, length);
//   }
//   fixedSynthetic(kReadHeavy, 0, length);
//   fixedSynthetic(kWritePartial, 0, length);

//   srand(time(0));
//   for (int i = 0; i < carmi_params::kDatasetSize; i++) {
//     length.push_back(
//         std::min(i + rand() % 100 + 1, carmi_params::kDatasetSize) - i);
//   }
//   fixedSynthetic(kRangeScan, 0, length);
// }

// void fixedSynthetic(double initRatio, int kLeafID,
//                     const std::vector<int> &length) {
//   std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&"
//                "&&&&&&&"
//             << std::endl;
//   std::cout << "initRatio is: " << initRatio << std::endl;
//   outRes << "initRatio," << initRatio << std::endl;
//   double init = initRatio;
//   if (init == kRangeScan) {
//     init = kReadHeavy;
//   }
//   LongitudesDataset longData = LongitudesDataset(init);
//   UniformDataset uniData = UniformDataset(init);
//   NormalDataset norData = NormalDataset(init);
//   ExponentialDataset expData = ExponentialDataset(init);

//   DataVecType initData;
//   DataVecType testInsert;

//   for (int i = 0; i < 1; i++) {
//     int childNum = 131072;
//     std::cout << "+++++++++++ uniform dataset ++++++++++++++++++++++++++"
//               << std::endl;
//     outRes << "+++++++++++ childNum: " << childNum << std::endl;
//     uniData.GenerateDataset(&initData, &testInsert);
//     outRes << "+++++++++++ uniform dataset++++++++++++++++++++++++++"
//            << std::endl;
//     RunStatic(initRatio, kLeafID, initData, testInsert, length);

//     std::cout << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
//               << std::endl;
//     outRes << "+++++++++++ childNum: " << childNum << std::endl;
//     expData.GenerateDataset(&initData, &testInsert);
//     outRes << "+++++++++++ exponential dataset ++++++++++++++++++++++++++"
//            << std::endl;
//     RunStatic(initRatio, kLeafID, initData, testInsert, length);

//     std::cout << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
//               << std::endl;
//     outRes << "+++++++++++ childNum: " << childNum << std::endl;
//     norData.GenerateDataset(&initData, &testInsert);
//     outRes << "+++++++++++ normal dataset ++++++++++++++++++++++++++"
//            << std::endl;
//     RunStatic(initRatio, kLeafID, initData, testInsert, length);

//     std::cout << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
//               << std::endl;
//     outRes << "+++++++++++ childNum: " << childNum << std::endl;
//     longData.GenerateDataset(&initData, &testInsert);
//     outRes << "+++++++++++ longitudes dataset ++++++++++++++++++++++++++"
//            << std::endl;
//     RunStatic(initRatio, kLeafID, initData, testInsert, length);
//   }
// }

// #endif  // SRC_STATICRMI_FIXED_EXPERIMENT_H_