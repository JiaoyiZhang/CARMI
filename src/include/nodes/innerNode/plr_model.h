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
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "./candidate_plr.h"

/**
 * @brief piecewise linear regression inner node
 *
 * This class is the p. lr inner node, which use a picewise linear regression to
 * train the model and predict the index of the next node. The CPU time cost of
 * this node is slightly more than the other nodes, but this node can deal with
 * the uneven dataset.
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
   * @param c [in] the number of its child nodes
   */
  explicit PLRModel(int c) {
    childLeft = 0;
    flagNumber = (PLR_INNER_NODE << 24) + c;
  }

 public:
  // *** Basic Functions of P. LR Inner Node Objects

  /**
   * @brief train the picewise linear regression model
   *
   * The training data points are stored in dataset[left, left + size].
   *
   * @param left[in] the starting index of data points
   * @param size[in]  the size of data points
   * @param dataset[in] used to train the model
   */
  void Train(int left, int size, const DataVectorType &dataset);

  /**
   * @brief predict the next node which manages the data point corresponding to
   * the given key value
   *
   * @param key[in] the given key value
   * @return int: the predicted index of next node
   */
  int Predict(KeyType key) const;

 public:
  // *** Static Constant Options and Values of P. LR Inner Node Objects

  /**
   * @brief The time cost of the p. lr inner node.
   */
  static constexpr int kTimeCost = carmi_params::kPLRInnerTime;

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
   * @brief store the maximum index of each segment, which is used as the
   * boundary of each segment when predicting the next index. (24 bytes)
   */
  int index[6];

  /**
   * @brief store the key value of the end point in each segment.
   * When predicting the index of the next node, first perform a binary search
   * among them to find the segment to which the given key value belongs (32
   * bytes)
   */
  float keys[8];
};

template <typename KeyType, typename ValueType>
inline void PLRModel<KeyType, ValueType>::Train(int left, int size,
                                                const DataVectorType &dataset) {
  int childNumber = flagNumber & 0x00FFFFFF;
  DataVectorType currdata(size, {-1, 0});
  int point_num = 7;

  keys[0] = dataset[left].first;
  keys[7] = dataset[left + size - 1].first;
  for (int i = 0, j = left; i < size; i++, j++) {
    currdata[i].second = static_cast<float>(i) / size * (childNumber - 1);
    currdata[i].first = dataset[j].first;
  }

  int cand_size = std::min(size, 100);
  DataVectorType cand_point(cand_size, {0, 0});
  std::vector<int> cand_index(cand_size, 0);
  CandidateCost<DataVectorType, DataType> cand_cost;
  float seg = size * 1.0 / cand_size;
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

  SegmentPoint tmp;
  tmp.cost = -DBL_MAX;
  std::vector<SegmentPoint> tmpp(cand_size, tmp);
  std::vector<std::vector<SegmentPoint>> dp(2, tmpp);

  for (int j = 1; j < cand_size; j++) {
    dp[0][j].cost = cand_cost.Entropy(0, cand_index[j]);
    dp[0][j].key[0] = cand_point[0].first;
    dp[0][j].idx[0] = cand_index[0];
    dp[0][j].key[1] = cand_point[j].first;
    dp[0][j].idx[1] = cand_index[j];
    dp[0][j].key[point_num] = cand_point[cand_size - 1].first;
    dp[0][j].idx[point_num] = cand_index[cand_size - 1];
  }

  for (int i = 2; i < point_num; i++) {
    for (int j = i + 1; j < cand_size - 1; j++) {
      SegmentPoint opt;
      opt.cost = -DBL_MAX;
      for (int k = i; k < j; k++) {
        double res = -DBL_MAX;
        if (i < point_num - 1) {
          res = dp[0][k].cost + cand_cost.Entropy(cand_index[k], cand_index[j]);
        } else {
          res = dp[0][k].cost +
                cand_cost.Entropy(cand_index[k], cand_index[j]) +
                cand_cost.Entropy(cand_index[j], size - 1);
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

  SegmentPoint opt;
  opt.cost = -DBL_MAX;
  opt.idx[point_num] = cand_index[cand_size - 1];
  opt.key[point_num] = cand_point[cand_size - 1].first;
  for (int j = point_num; j < cand_size - 1; j++) {
    if (dp[1][j].cost > opt.cost) {
      opt.cost = dp[1][j].cost;
      for (int k = 0; k < point_num; k++) {
        opt.idx[k] = dp[1][j].idx[k];
        opt.key[k] = dp[1][j].key[k];
      }
    }
  }
  for (int i = 1; i < point_num; i++) {
    keys[i] = opt.key[i];
    index[i - 1] = currdata[opt.idx[i]].second;
  }
}

template <typename KeyType, typename ValueType>
inline int PLRModel<KeyType, ValueType>::Predict(KeyType key) const {
  int s = 0;
  int e = 7;
  int mid;
  // first perform a binary search among the keys
  while (s < e) {
    mid = (s + e) >> 1;
    if (keys[mid] < key)
      s = mid + 1;
    else
      e = mid;
  }

  if (e == 0) {
    return 0;
  }

  int p;
  // use the params of the segment to predict the index of the next node
  if (e == 7) {
    e = (flagNumber & 0x00FFFFFF) - 1;                   // boundary
    float slope = (e - index[5]) / (keys[7] - keys[6]);  // the slope
    // intercept = e - slope * keys[7], p = slope * x + intercept
    p = slope * (key - keys[7]) + e;

    if (p > e) {
      return e;
    } else if (p < 0) {
      return 0;
    }

  } else if (e == 1) {
    float slope = static_cast<float>(index[0]) / (keys[1] - keys[0]);
    // intercept = -slope * keys[0], p = slope * x + intercept
    p = slope * (key - keys[0]);
    if (p < 0) {
      p = 0;
    } else if (p > index[0]) {
      p = index[0];
    }
  } else {
    float slope = static_cast<float>(index[e - 1] - index[e - 2]) /
                  (keys[e] - keys[e - 1]);
    // intercept = index[e-1] - slope * keys[e]
    p = slope * (key - keys[e]) + index[e - 1];
    if (p < index[e - 2]) {
      p = index[e - 2];
    } else if (p > index[e - 1]) {
      p = index[e - 1];
    }
  }
  return p;
}

#endif  // NODES_INNERNODE_PLR_MODEL_H_
