// /**
//  * @file static.h
//  * @author Jiaoyi
//  * @brief
//  * @version 0.1
//  * @date 2021-03-16
//  *
//  * @copyright Copyright (c) 2021
//  *
//  */
// #ifndef SRC_STATICRMI_STATIC_H_
// #define SRC_STATICRMI_STATIC_H_
// #include <vector>

// #include "../experiment/functions.h"
// #include "../include/carmi_common.h"
// #include "./rmi.h"

// extern int childNum;
// extern std::ofstream outRes;

// void RunStatic(double initRatio, int kLeafID,
//                const DataVecType &initDataset,
//                const DataVecType &testInsertQuery,
//                const std::vector<int> &length) {
//   for (int j = 2; j < 3; j++) {
//     std::cout << "root type:" << j << std::endl;

//     typedef CARMICommon<KeyType, ValueType>
//         CarmiType;
//     CarmiType carmi(initDataset, 131072, j, kLeafID);
//     std::cout << "index init over!" << std::endl;
//     switch (j) {
//       case LR_ROOT_NODE:
//         outRes << "lr,";
//         break;
//       case PLR_ROOT_NODE:
//         outRes << "plr,";
//         break;
//       case HIS_ROOT_NODE:
//         outRes << "his,";
//         break;
//       case BS_ROOT_NODE:
//         outRes << "bin,";
//         break;
//     }

//     if (initRatio == kWriteHeavy)
//       WorkloadA<KeyType, ValueType>(
//           false, initDataset, testInsertQuery,
//           &carmi);  // write-heavy
//     else if (initRatio == kReadHeavy)
//       WorkloadB<KeyType, ValueType>(
//           false, initDataset, testInsertQuery,
//           &carmi);  // read-heavy
//     else if (initRatio == kReadOnly)
//       WorkloadC<KeyType, ValueType>(
//           false, initDataset,
//           &carmi);  // read-only
//     else if (initRatio == kWritePartial)
//       WorkloadD<KeyType, ValueType>(
//           false, initDataset, testInsertQuery,
//           &carmi);  // write-partial
//     else if (initRatio == kRangeScan)
//       WorkloadE<KeyType, ValueType>(
//           false, initDataset, testInsertQuery, length,
//           &carmi);  // range scan
//   }
//   outRes << std::endl;
// }

// #endif  // SRC_STATICRMI_STATIC_H_
