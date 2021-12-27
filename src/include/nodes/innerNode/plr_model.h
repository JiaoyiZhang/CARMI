/**
 * @file plr_model.h
 * @author Jiaoyi
 * @brief piecewise linear regression inner node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_INNERNODE_PLR_MODEL_H_
#define NODES_INNERNODE_PLR_MODEL_H_

#include <float.h>

#include <algorithm>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "./candidate_plr.h"

/**
 * @brief piecewise linear regression inner node
 *
 * This class is the p. lr inner node, which uses a piecewise linear regression
 * to train the model and predict the index of the next node. The CPU time cost
 * of this node is slightly more than the other nodes, but this node can deal
 * with the uneven dataset.
 *
 * @tparam KeyType the type of the keyword
 * @tparam ValueType the type of the value
 */
template <typename KeyType, typename ValueType>
class PLRModel {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief the pair of data points
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief the vector of data points, which is the type of dataset
   */
  typedef std::vector<DataType> DataVectorType;

  /**
   * @brief Construct a new PLRModel object and use c to set its child number
   *
   * @param[in] c  the number of its child nodes
   */
  explicit PLRModel(int c) {
    childLeft = 0;
    minValue = 0;
    for (int i = 0; i < kIndexNum; i++) {
      index[i] = std::ceil(c / static_cast<double>(kSegmentNum)) * (i + 1);
    }
    for (int i = 0; i < kSegmentNum + 1; i++) {
      keys[i] = i;
    }
    flagNumber = (PLR_INNER_NODE << 24) + std::max(std::min(c, 0x00FFFFFF), 2);
  }

 public:
  // *** Basic Functions of P. LR Inner Node Objects

  /**
   * @brief train the picewise linear regression model
   *
   * The training data points are stored in dataset[left, left + size].
   *
   * @param[in] left the starting index of data points
   * @param[in] size  the size of data points
   * @param[in] dataset used to train the model, each data point in the dataset
   * is composed of the key value and the value.
   */
  void Train(int left, int size, const DataVectorType &dataset);

  /**
   * @brief predict the next node which manages the data point corresponding to
   * the given key value
   *
   * @param[in] key the given key value
   * @return int: the predicted index of next node
   */
  int Predict(KeyType key) const;

 public:
  // *** Static Constant Options and Values of P. LR Inner Node Objects

  /**
   * @brief The time cost of the p. lr inner node.
   */
  static constexpr double kTimeCost = carmi_params::kPLRInnerTime;

  /**
   * @brief The number of index to be stored.
   */
  static constexpr int kIndexNum =
      (56 - 3 * sizeof(KeyType)) / (4 + sizeof(KeyType));

  /**
   * @brief The bytes of placeholder.
   */
  static constexpr int kPlaceHolderLen =
      56 - 3 * sizeof(KeyType) - (4 + sizeof(KeyType)) * kIndexNum;

  /**
   * @brief The number of segments.
   */
  static constexpr int kSegmentNum = kIndexNum + 1;

 public:
  //*** Public Data Members of P. LR Inner Node Objects

  /**
   * @brief A combined integer, composed of the flag of p. lr inner node
   * (PLR_INNER_NODE, 1 byte) and the number of its child nodes (3 bytes). (This
   * member is 4 bytes)
   */
  int flagNumber;

  /**
   * @brief The index of its first child node in the node array. All the child
   * nodes are stored in node[childLeft, childLeft + size]. Through this member
   * and the right three bytes of flagNumber, all the child nodes can be
   * accessed. (4 bytes)
   */
  int childLeft;

  /**
   * @brief Store the maximum index of each segment, which is used as the
   * boundary of each segment when predicting the next index. (kIndexNum * 4
   * bytes)
   */
  int index[kIndexNum];

  /**
   * @brief Store the key value of the endpoint in each segment. When predicting
   * the index of the next node, first perform a binary search among them to
   * find the segment to which the given key value belongs ((kIndexNum + 2) *
   * sizeof(KeyType) bytes)
   */
  KeyType keys[kIndexNum + 2];

  /**
   * @brief The minimum value.
   */
  KeyType minValue;

  /**
   * @brief Placeholder.
   */
  char placeholder[kPlaceHolderLen];
};

template <typename KeyType, typename ValueType>
inline void PLRModel<KeyType, ValueType>::Train(int left, int size,
                                                const DataVectorType &dataset) {
  int childNumber = flagNumber & 0x00FFFFFF;
  if (size == 0) {
    return;
  }
  if (left < 0 || size < 0 || left + size > dataset.size()) {
    throw std::out_of_range(
        "PLRModel::Train: the range of training dataset is invalid.");
  }
  typedef std::vector<std::pair<KeyType, double>> TrainType;
  TrainType currdata(size);

  minValue = dataset[left].first;
  keys[0] = dataset[left].first - minValue;
  keys[kSegmentNum] = dataset[left + size - 1].first - minValue;
  // construct the training dataset, x is the key value in the dataset, y is the
  // corresponding ratio of index in the childNumber
  for (int i = 0, j = left; i < size; i++, j++) {
    currdata[i].first = dataset[j].first - minValue;
    currdata[i].second = static_cast<float>(i) / size * (childNumber - 1);
  }

  int cand_size = std::min(size, 100);
  // store the index and data points of candidate points into cand_index and
  // cand_point to speed up the dp algorithm
  TrainType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost<TrainType> cand_cost;
  float seg = static_cast<double>(size) / cand_size;
  for (int i = 0; i < cand_size - 1; i++) {
    if (i * seg >= size) {
      for (; i < cand_size - 1; i++) {
        cand_index[i] = size - 1;
        cand_point[i] = currdata[size - 1];
      }
      break;
    }
    cand_index[i] = i * seg;
    cand_point[i] = currdata[i * seg];
  }
  cand_index[cand_size - 1] = size - 1;
  cand_point[cand_size - 1] = currdata[size - 1];
  cand_cost.StoreTheta(currdata, cand_index);

  std::vector<std::vector<SegmentPoint<KeyType>>> dp(
      2,
      std::vector<SegmentPoint<KeyType>>(cand_size, SegmentPoint<KeyType>()));

  // initialize the dp[0]
  for (int j = 1; j < cand_size; j++) {
    dp[0][j].cost =
        cand_cost.Entropy(0, cand_index[j], 0, cand_point[j].second);
    dp[0][j].key[0] = cand_point[0].first;
    dp[0][j].idx[0] = cand_index[0];
    dp[0][j].key[1] = cand_point[j].first;
    dp[0][j].idx[1] = cand_index[j];
    dp[0][j].key[kSegmentNum] = cand_point[cand_size - 1].first;
    dp[0][j].idx[kSegmentNum] = cand_index[cand_size - 1];
  }

  // use dp algorithm to calculate the optimal cost in each situation
  for (int i = 2; i < kSegmentNum; i++) {
    for (int j = i; j < cand_size - 1; j++) {
      SegmentPoint<KeyType> opt;
      for (int k = i - 1; k < j; k++) {
        float res = -DBL_MAX;
        if (i < kSegmentNum - 1) {
          res = dp[0][k].cost + cand_cost.Entropy(cand_index[k], cand_index[j],
                                                  cand_point[k].second,
                                                  cand_point[j].second);
        } else {
          res = dp[0][k].cost +
                cand_cost.Entropy(cand_index[k], cand_index[j],
                                  cand_point[k].second, cand_point[j].second) +
                cand_cost.Entropy(cand_index[j], size - 1, cand_point[j].second,
                                  cand_point[cand_size - 1].second);
        }
        if (res > opt.cost) {
          opt.cost = res;
          for (int l = 0; l < i; l++) {
            opt.idx[l] = dp[0][k].idx[l];
            opt.key[l] = dp[0][k].key[l];
          }
          opt.key[i] = cand_point[j].first;
          opt.idx[i] = cand_index[j];
        }
      }
      dp[1][j].cost = opt.cost;
      for (int l = 0; l <= i; l++) {
        dp[1][j].idx[l] = opt.idx[l];
        dp[1][j].key[l] = opt.key[l];
      }
    }
    for (int m = 0; m < cand_size; m++) {
      dp[0][m].cost = dp[1][m].cost;
      for (int l = 0; l <= i + 1; l++) {
        dp[0][m].idx[l] = dp[1][m].idx[l];
        dp[0][m].key[l] = dp[1][m].key[l];
      }
    }
  }

  // find the optimal setting and store the params into keys and index
  SegmentPoint<KeyType> opt;
  opt.idx[kSegmentNum] = cand_index[cand_size - 1];
  opt.key[kSegmentNum] = cand_point[cand_size - 1].first;
  for (int j = kSegmentNum; j < cand_size - 1; j++) {
    if (dp[1][j].cost > opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < kSegmentNum; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    }
  }
  for (int i = 1; i < kSegmentNum; i++) {
    if (opt.key[i] == -1) {
      opt.key[i] = opt.key[i - 1];
      opt.idx[i] = opt.idx[i - 1];
    }
    keys[i] = opt.key[i];
    index[i - 1] = currdata[opt.idx[i]].second;
  }
}

template <typename KeyType, typename ValueType>
inline int PLRModel<KeyType, ValueType>::Predict(KeyType key) const {
  KeyType tmpKey = key - minValue;
  int s = 0;
  int e = kSegmentNum;
  int mid;
  // first perform a binary search among the keys
  while (s < e) {
    mid = (s + e) >> 1;
    if (keys[mid] < tmpKey)
      s = mid + 1;
    else
      e = mid;
  }

  int p = 0;
  switch (e) {
    case 0: {
      break;
    }
    case 1: {
      float slope = static_cast<double>(index[0]) / (keys[1] - keys[0]);
      // intercept = -slope * keys[0], p = slope * x + intercept
      p = slope * (tmpKey - keys[0]);
      if (p < 0) {
        p = 0;
      } else if (p > index[0]) {
        p = index[0];
      }
      break;
    }
    case kSegmentNum: {
      e = (flagNumber & 0x00FFFFFF) - 1;  // boundary
      float slope = static_cast<double>(e - index[kIndexNum - 1]) /
                    static_cast<double>(keys[kSegmentNum] -
                                        keys[kIndexNum]);  // the slope
      // intercept = e - slope * keys[7], p = slope * x + intercept
      p = slope * static_cast<double>(tmpKey - keys[kSegmentNum]) + e;

      if (p > e) {
        p = e;
      } else if (p < index[kIndexNum - 1]) {
        p = index[kIndexNum - 1];
      }
      break;
    }
    default: {
      float slope = static_cast<double>(index[e - 1] - index[e - 2]) /
                    static_cast<double>(keys[e] - keys[e - 1]);
      // intercept = index[e-1] - slope * keys[e]
      p = slope * static_cast<double>(tmpKey - keys[e]) + index[e - 1];
      if (p < index[e - 2]) {
        p = index[e - 2];
      } else if (p > index[e - 1]) {
        p = index[e - 1];
      }
      break;
    }
  }
  return p + childLeft;
}

#endif  // NODES_INNERNODE_PLR_MODEL_H_
