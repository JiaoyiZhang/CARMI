/**
 * @file carmi.h
 * @author Jiaoyi
 * @brief the implementation of CARMI
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef CARMI_H_
#define CARMI_H_

#include <float.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "./base_node.h"
#include "./params.h"
#include "construct/structures.h"
#include "memoryLayout/data_array.h"
#include "memoryLayout/node_array.h"

#define log2(value) log(value) / log(2)

/**
 * @brief The main class of the cache-aware learned index framework, called
 * CARMI.
 *
 * CARMI uses a hybrid construction algorithm to automatically construct the
 * optimal index structures under various datasets without any manual tuning.
 *
 * This class contains the specific implementation of all the basic functions of
 * the CARMI framework, including the basic operations and construction of the
 * index, and supports two different types of CARMI: (1) CARMI that needs to
 * store and manage data points in the index structure and (2) External CARMI
 * that only stores the pointer of the position of external data points in the
 * index structure
 *
 * @tparam KeyType Type of keys.
 * @tparam ValueType Type of values of data points.
 * @tparam Compare A binary predicate that takes two element keys as arguments
 * and returns a bool.
 * @tparam Alloc Type of the allocator object used to define the storage
 * allocation model.
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
class CARMI {
 public:
  // *** Constructed Types and Constructor

  /**
   * @brief The pair of data points: {key, value}
   */
  typedef std::pair<KeyType, ValueType> DataType;

  /**
   * @brief The vector of data points, which is the type of dataset: [{key_0,
   * value_0}, {key_1, value_1}, ..., {key_n, value_n}]. The dataset has been
   * sorted.
   */
  typedef std::vector<DataType> DataVectorType;

  /**
   * @brief The type of the key value vector: [key_0, key_1, ..., key_n].
   */
  typedef std::vector<KeyType> KeyVectorType;

  /**
   * @brief The type of the historical queries vector: [{key_0, times_0},
   * {key_1, times_1}, ..., {key_n, times_n}]. The second object means the
   * accessed times of the key value in the historical access queries.
   */
  typedef std::vector<std::pair<KeyType, int>> QueryType;

  Compare key_less_ = Compare();
  Alloc allocator_ = Alloc();

 public:
  // *** Key and Value Comparison Function Objects

  /**
   * @brief Gets the allocator object.
   *
   * @return Alloc
   */
  Alloc get_allocator() const { return allocator_; }

  /**
   * @brief Returns a copy of the comparison object used by the container
   * to compare keys.
   *
   * @return Compare
   */
  Compare key_comp() const { return key_less_; }

  /**
   * @brief A class that uses the internal comparison object to generate
   * the appropriate comparison functional class.
   */
  class value_compare {
   protected:
    Compare comp;
    explicit value_compare(Compare c) : comp(c) {}

   public:
    /**
     * @brief Compare two elements to get whether the key of the first one
     * goes before the second.
     *
     * @param[in] x the first object
     * @param[in] y the second object
     * @retval true the key of the first argument is considered to go
     * before that of the second
     * @retval false the key of the first argument is considered to go
     * after that of the second
     */
    bool operator()(const DataType &x, const DataType &y) const {
      return comp(x.first, y.first);
    }
  };

  /**
   * @brief Returns a comparison object that can be used to compare two
   * elements to get whether the key of the first one goes before the
   * second.
   *
   * @return value_compare The comparison object for element values.
   */
  value_compare value_comp() const { return value_compare(key_less_); }

 public:
  // *** Constructors and Destructor

  /**
   * @brief Construct a new empty CARMI object
   */
  CARMI();

  /**
   * @brief Construct a new CARMI object, which uses a hybrid construction
   * algorithm to automatically construct the optimal index structures under
   * various datasets without any manual tuning. This construction function is
   * designed for the dataset that needs to be indexed and its corresponding
   * historical queries. If the user does not have historical queries, then
   * findQuery and insertQuery can directly pass in empty vectors of this type,
   * and this function will default to a read-only workload to construct the
   * optimal index structure.
   *
   * First, assign initial values ​​to some basic variables, and then use
   * the given three datasets to build the optimal index structure tree.
   *
   * (1) Among them, initDataset is used to train the model in each node,
   * representing the data points that have been stored before in the database
   * in the historical queries, and each element of it is a pair of data points:
   * {key, value}. Therefore, during constructing the index, the data points of
   * initDataset will be actually stored in the part of the index tree
   * responsible for managing data points.
   *
   * (2) findQuery represents the key values of all data points that have been
   * accessed in the history queries and the number of times it has been
   * visited. Each element is a pair: {key value of this data point, the visited
   * times}. This dataset is used to calculate the cost model.
   *
   * (3) Similarly, insertQuery represents the data points inserted in the
   * history queries. Each element is a pair: {key value of this data point, the
   * inserted times}. In the future, there may be similar values ​​inserted
   * at similar positions. Therefore, in addition to calculating the cost model,
   * this dataset is also used to reserve the space for future insert operations
   * in a similar location.
   *
   * (4) The last parameter lambda is used to tradeoff between the time and
   * space cost in the cost model. The CARMI index structure can achieve good
   * performance under this time/space setting. The goal of the construction
   * algorithm is to minimize the total cost of historical access and insert
   * queries, which is: the average time cost of each query + lambda times the
   * space cost of the index tree.
   *
   * @param[in] initData the vector of init dataset, used to train models and
   * construct the index tree. Each element is: {key, value}
   * @param[in] findData the vector of find dataset, used as historical find
   * queries to assist in constructing index. Each element is: {key value,
   * the visited times of this data point}
   * @param[in] insertData the key vector of insert dataset, used as historical
   * insert queries to assist in constructing index. Each element is the key
   * value to be inserted.
   * @param[in] lambda cost = time + lambda * space, used to tradeoff between
   * time and space cost
   */
  CARMI(const DataVectorType &initData, const QueryType &findData,
        const KeyVectorType &insertData, double lambda);

  /**
   * @brief Construct a new CARMI object, which uses a hybrid construction
   * algorithm to automatically construct the optimal index structures under
   * various datasets without any manual tuning.
   *
   * This function is designed for the primary indexes that store data points
   * externally. In other words, in this case, the CARMI index structure will
   * not actually store these data points, but only store the position of the
   * first data point of the dataset. Users pass in three parameters: a pointer
   * to an external dataset, the length of a record, and the total number of
   * records to represent this dataset. This function will automatically
   * generate the initDataset, findQuery, and insertQuery required by the
   * algorithm based on the parameters.
   *
   * (1) We can obtain the initDataset from the pointer to the dataset and store
   * them in the form of a vector, each element is a pair of data points: {key,
   * value}. initDataset is used to train the model in each node, representing
   * the data points that have been stored before in the database in the
   * historical queries.
   *
   * (2) findQuery represents the key values of all data points that have been
   * accessed in the history queries and the number of times it has been
   * visited. Each element is a pair: {key value of this data point, the visited
   * times}. This dataset is used for the calculation of the cost model. Due to
   * the particularity of storing the dataset externally, we default that each
   * data point in initDataset is accessed once to generate this dataset.
   *
   * (3) Similarly, insertQuery represents the data points inserted in the
   * history queries. Each element is the key value. In the future, there may be
   * similar values ​​inserted at similar positions. Therefore, in addition
   * to calculating the cost model, insertQuery is also used to reserve the
   * space for future insert operations in a similar location.
   *
   * (4) The third parameter lambda is used to tradeoff between the time and
   * space cost in the cost model. The CARMI index structure can achieve good
   * performance under this time/space setting. The goal of the construction
   * algorithm is to minimize the total cost of historical access and insert
   * queries, which is: the average time cost of each query + lambda times the
   * space cost of the index tree.
   *
   * (5) record_number refers to the number of all records in the external
   * dataset. A key value and its corresponding payload form a record.
   *
   * (6) record_len refers to the length of this record, including the key value
   * and payload, in bytes. For example, if the key value of a record in an
   * external dataset is an object of type double, and the value is ten objects
   * of type float, then the length of the data record is 48 bytes.
   *
   * @param[in] dataset the pointer of the dataset
   * @param[in] insertData the vector of insert dataset, used as historical
   * insert queries to assist in constructing index. Each element is: {key
   * value, the inserted times of this data point}
   * @param[in] lambda cost = time + lambda * space, used to tradeoff between
   * time and space cost
   * @param[in] record_number the number of all the records
   * @param[in] record_len the length of a record, in bytes (including the key
   * value and the payload)
   */
  CARMI(const void *dataset, const KeyVectorType &insertData, double lambda,
        int record_number, int record_len);

  /**
   * @brief Removes all elements
   */
  void clear() {
    data = DataArrayStructure<KeyType, ValueType, Alloc>(
        CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxBlockNum, 1000);
    node = NodeArrayStructure<KeyType, ValueType, Compare, Alloc>();
  }

  /**
   * @brief The main function of index construction.
   *
   * This function will be called after the specific implementation of CARMI
   * completes data preprocessing and other operations, and uses the
   * corresponding parameters to construct an index structure with good
   * performance for the given dataset.
   */
  void Construction();

 public:
  // *** Basic Public Functions of CARMI Objects

  /**
   * @brief Find a data point of the given key value and return its position.
   *
   * @param[in] key the given key value
   * @param[out] currblock the index of the data block of the returned leaf
   * node, useless in the external array leaf node
   * @param[out] currslot (1) the index of the data point in the data block in
   * the cf array leaf node; (2) the index of the data point in the external
   * array leaf node
   * @return BaseNode<KeyType, ValueType, Compare, Alloc>*: the leaf node which
   * manages this data point
   */
  BaseNode<KeyType, ValueType, Compare, Alloc> *Find(const KeyType &key,
                                                     int *currblock,
                                                     int *currslot);

  /**
   * @brief Insert a data point.
   *
   * @param[in] datapoint the inserted data point
   * @param[out] currblock the index of the data block of the returned leaf
   * node, useless in the external array leaf node
   * @param[out] currslot (1) the index of the data point in the data block in
   * the cf array leaf node; (2) the index of the data point in the external
   * array leaf node
   * @return std::pair<BaseNode<KeyType, ValueType, Compare, Alloc>*, bool> the
   * first element is the leaf node which manages this data point and the
   * pair::second element in the pair is set to true if a new element was
   * inserted.
   */
  std::pair<BaseNode<KeyType, ValueType, Compare, Alloc> *, bool> Insert(
      const DataType &datapoint, int *currblock, int *currslot);

  /**
   * @brief Delete all the records of the given key.
   *
   * @param[in] key the key value of the deleted record
   * @param[out] cnt the number of deleted data points
   * @retval true deletion is successful
   */
  bool Delete(const KeyType &key, size_t *cnt);

  /**
   * @brief Delete a data point.
   *
   * @param[in] key the key value of the deleted record
   * @param[in] currnode the leaf node
   * @param[in] currblock the index of the data block of the leaf node
   * @param[in] currslot the index of the data point in the data block in
   * the cf array leaf node
   * @retval true if the operation succeeds
   * @retval false if the operation fails (the given position is invalid)
   */
  bool DeleteSingleData(
      const KeyType &key,
      const BaseNode<KeyType, ValueType, Compare, Alloc> &currnode,
      int currblock, int currslot) {
    if (currblock < 0 || currslot < 0 ||
        currblock >= (currnode->cfArray.flagNumber & 0x00FFFFFF) ||
        currslot >= currnode->cfArray.GetBlockSize(currblock)) {
      return true;
    }
    return currnode.DeleteSingleData(key, currblock, currslot, &data);
  }

 public:
  // *** Functions of Getting Some Information of CARMI Objects

  /**
   * @brief Calculate the space cost of the CARMI object.
   *
   * @return long long: the space cost (byte)
   */
  long long CalculateSpace() const;

  /**
   * @brief Get the information of the tree node, return the type identifier of
   * this node, the number of its child nodes and the starting index of the
   * first child node in the node array.
   *
   * @param[in] idx the index of the node in the node array
   * @param[out] childNumber the number of the child nodes of this node
   * @param[out] childStartIndex the starting index of the first child node
   * @return int the type identifier of this node
   */
  int GetNodeInfo(int idx, int *childNumber, int *childStartIndex);

 private:
  //*** Private Functions of Constructing Root Nodes of CARMI Objects

  /**
   * @brief Update the optimal setting of the root node.
   *
   * Generate a temporary root node according to the current root node setting
   * and calculate the corresponding time cost, space cost and entropy to get
   * the cost of this setting. If the cost is less than the optimal cost, update
   * the optimal cost value and the optimal root node setting, otherwise return
   * directly.
   *
   * Space cost is the total space cost of all the child nodes.
   * Time cost is the time cost of the root node plus the weighted time cost of
   * all inner nodes.
   * Entropy is the sum of -p_j log_2(p_j) value of all child nodes
   *
   * @tparam RootNodeType the typename of this node
   * @param[in] c the child number of this node
   * @param[inout] optimalCost the optimal cost
   * @param[inout] rootStruct the optimal cost setting: {the number of the child
   * nodes, the type of the root node}
   */
  template <typename RootNodeType>
  void UpdateRootOptSetting(int c, double *optimalCost, RootStruct *rootStruct);

  /**
   * @brief The main function of selecting the optimal root node setting.
   *
   * This function will generate many different combinations of node settings as
   * the input of the UpdateRootOptSetting function, find the optimal root node
   * from them, and return the optimal root node.
   *
   * @return the type and childNumber of the optimal root: {the number of the
   * child nodes, the type of the root node}
   */
  RootStruct ChooseRoot();

  /**
   * @brief The main function to store the root node.
   *
   * This function constructs the root node based on the optimal root node
   * setting selected by the ChooseRoot function. It divides the dataset with
   * the model of the root node to prepare for the construction of its child
   * nodes.
   *
   * @param[in] rootStruct the optimal setting of the root node: {the number of
   * the child nodes, the type of the root node}
   * @return SubDataset: the starting index and size of sub-dataset in each
   * child node, each element is: {the vector of the sub-initDataset, the vector
   * of the sub-findDataset, the vector of sub-insertDataset}. Each sub-dataset
   * is represented by: {left, size}, which means the range of it in the dataset
   * is [left, left + size).
   */
  SubDataset StoreRoot(const RootStruct &rootStruct);

 private:
  //*** Main Private Function of Constructing Child Nodes of the Root Node

  /**
   * @brief The main function of constructing each subtree using the DP/greedy
   * algorithm.
   *
   * This function recursively constructs the optimal sub-index tree for each
   * child node of the root node and its corresponding sub-dataset, and stores
   * them in the corresponding node and data arrays. At this point, since the
   * root node has already been determined, we can train the prefetch prediction
   * model of the root node. When constructing leaf nodes, we try to store data
   * points according to the output of the prefetch prediction model as much as
   * possible.
   *
   * @param[in] subDataset the starting index and size of sub-dataset in each
   * child node, each element is: {the vector of the sub-initDataset, the vector
   * of the sub-findDataset, the vector of sub-insertDataset}. Each sub-dataset
   * is represented by: {left, size}, which means the range of it in the dataset
   * is [left, left + size).
   */
  void ConstructSubTree(const SubDataset &subDataset);

 private:
  //*** Private Functions of Dynamic Programming Algorithm

  /**
   * @brief The main function of the dynamic programming algorithm to construct
   * the optimal sub-index tree.
   *
   * This function is the external interface of the dynamic programming
   * algorithm. It compares the size of the dataset with the parameters, chooses
   * an appropriate algorithm to construct a leaf node, an inner node, or both,
   * and chooses the better one from them. Finally, the cost of the sub-tree
   * corresponding to the sub-dataset is returned.
   *
   * @param[in] range the range of the sub-dataset: {{the left index in the
   * initDataset, the size of the sub-initDataset}, {the left index in the
   * findQuery, the size of the sub-findQuery}, {the left index in the
   * insertQuery, the size of the sub-insertQuery}}
   * @return NodeCost: the cost of the subtree: {time cost, space cost, total
   * cost}
   */
  NodeCost DP(const DataRange &range);

  /**
   * @brief Traverse all possible settings to find the optimal inner node.
   *
   * This function generates various inner node settings according to the given
   * sub-dataset, then calls the UpdateDPOptSetting function to calculate their
   * costs, select the optimal settings for this sub-dataset, and return the
   * minimum cost.
   *
   * @param[in] range the range of the sub-dataset: {initDataset: {left, size},
   * findQuery: {left, size}, insertQuery: {left, size}}
   * @return NodeCost: the optimal cost of this subtree: {time cost, space cost,
   * total cost}
   */
  NodeCost DPInner(const DataRange &dataRange);

  /**
   * @brief Construct a leaf node directly.
   *
   * This function constructs a leaf node directly as the current node and
   * returns its cost. Based on our current implementation, the type of the leaf
   * node depends on the isPrimary parameter. If it is true, construct an
   * external array leaf node, otherwise, construct a cache-friendly array leaf
   * node.
   *
   * @param[in] range the range of the sub-dataset: {initDataset: {left, size},
   * findQuery: {left, size}, insertQuery: {left, size}}
   * @return NodeCost: the optimal cost of this leaf node: {time cost, space
   * cost, total cost}
   */
  NodeCost DPLeaf(const DataRange &dataRange);

  /**
   * @brief Update the optimal setting of the inner node using the dynamic
   * programming algorithm.
   *
   * This function uses the parameters passed by DPInner to construct the
   * current inner node and uses the cost model to calculate the cost of the
   * subtree, and returns the optimal node setting and the minimum cost. This
   * function will recursively call DP and GreedyAlgorithm functions to
   * construct sub-structures respectively.
   *
   * Root cost = frequency_weight * time cost of this inner node + lambda *
   * space cost of this inner node
   *
   * Total cost = the sum cost of all its child nodes + root cost
   *
   * @tparam InnerNodeType the type of this inner node
   * @param[in] dataRange the range of the sub-dataset: {initDataset: {left,
   * size}, findQuery: {left, size}, insertQuery: {left, size}}
   * @param[in] c the child number of this inner node
   * @param[in] frequency_weight the frequency weight of these queries
   * @param[inout] optimalCost the optimal cost: {time cost, space
   * cost, total cost}
   * @param[inout] optimal_node_struct the optimal setting
   */
  template <typename InnerNodeType>
  void UpdateDPOptSetting(const DataRange &dataRange, int c,
                          double frequency_weight, NodeCost *optimalCost,
                          InnerNodeType *optimal_node_struct);

 private:
  //*** Private Functions of Greedy Node Selection Algorithm

  /**
   * @brief The main function of the greedy node selection algorithm to
   * construct the optimal sub-index tree.
   *
   * This function is the external interface of the greedy node selection
   * algorithm. It uses the local information to construct the current inner
   * node. The cost of the sub-tree corresponding to the sub-dataset is
   * returned.
   *
   * @param[in] dataRange the range of the sub-dataset: {initDataset: {left,
   * size}, findQuery: {left, size}, insertQuery: {left, size}}
   * @return NodeCost: the cost of the subtree: {time cost, space
   * cost, total cost}
   */
  NodeCost GreedyAlgorithm(const DataRange &dataRange);

  /**
   * @brief Update the optimal setting of the inner node using the greedy node
   * selection algorithm for the range of the sub-dataset.
   *
   * Generate a temporary inner node with c child nodes according to the current
   * node setting and calculate the corresponding time cost, space cost and
   * entropy to get the cost of this setting. If the cost is less than the
   * optimal cost, update the optimal cost value and the optimal node struct,
   * otherwise, return directly.
   *
   * cost = (time_cost + lambda * space_cost / frequency_weight) / entropy
   *
   * @tparam InnerNodeType the type of this inner node
   * @param[in] range the range of the sub-dataset: {initDataset: {left,
   * size}, findQuery: {left, size}, insertQuery: {left, size}}
   * @param[in] c the child number of this inner node
   * @param[in] frequency_weight the frequency weight of these queries
   * @param[inout] optimalCost the optimal cost: {time cost, space
   * cost, total cost}
   * @param[inout] optimal_node_struct the optimal setting
   */
  template <typename InnerNodeType>
  void UpdateGreedyOptSetting(const DataRange &range, int c,
                              double frequency_weight, NodeCost *optimalCost,
                              InnerNodeType *optimal_node_struct);

 private:
  //*** Private Functions of Storing Tree Nodes

  /**
   * @brief The main function of storing the optimal tree nodes.
   *
   * This function uses the range of the initDataset to find the optimal setting
   * of the current node and recursively stores its child nodes, then stores
   * this node in the storeIdx-th position of the node array.
   *
   * @param[in] range the range of data points: {initDataset: {left,
   * size}, findQuery: {left, size}, insertQuery: {left, size}}
   * @param[in] storeIdx the index of this node stored in the node array
   */
  void StoreOptimalNode(const DataRange &range, int storeIdx);

  /**
   * @brief Divide the dataset using the model of this inner node and
   * recursively call the StoreOptimalNode function to store its child node, and
   * finally update its childLeft parameter.
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] range the range of the sub-dataset
   * @param[inout] node the inner node to be stored, the left index of its child
   * nodes in the node array will be updated in this function
   */
  template <typename InnerNodeType>
  void StoreInnerNode(const IndexPair &range, InnerNodeType *node);

 private:
  //*** Private Minor Functions to Favour the Above Functions

  /**
   * @brief Split the current leaf node into an inner node and several leaf
   * nodes.
   *
   * This function will be triggered when the leaf node cannot accommodate more
   * data points. It constructs an inner node and multiple leaf nodes to manage
   * the sub-dataset, and completely replaces the previous leaf node.
   *
   * @tparam LeafNodeType the type of the current leaf node
   * @param[in] idx the index of the current leaf node in the node array
   */
  template <typename LeafNodeType>
  void Split(int idx);

  /**
   * @brief Calculate the frequency weight of the sub-dataset whose range is
   * represented by dataRange.
   *
   * @param[in] dataRange the range of data points: {initDataset: {left,
   * size}, findQuery: {left, size}, insertQuery: {left, size}}
   * @return double: frequency weight
   */
  double CalculateFrequencyWeight(const DataRange &dataRange);

  /**
   * @brief Calculate the entropy of this node.
   *
   * Entropy = the sum of -pi*log(pi), where pi is the ratio of the size of each
   * sub-dataset in the total size
   *
   * @param[in] perSize the size of each child node of this node, each element
   * is a pair: {the left index in the initDataset, the size of the sub-dataset}
   * @return double: entropy
   */
  double CalculateEntropy(const std::vector<IndexPair> &perSize) const;

  /**
   * @brief Calculate the costs of the cf array node in different numbers of
   * allocated data blocks.
   *
   * @param[in] size the size of these data points
   * @param[in] totalPrefetchedNum the total size of the data points which can
   * be prefetched
   * @return std::vector<double> the vector of all cost, each element is the
   * total cost: time + lambda * space
   */
  std::vector<double> CalculateCFArrayCost(int size, int totalPrefetchedNum);

  /**
   * @brief Use this node to split the data points of the given dataset and
   * return the range of each sub-dataset.
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] node the current node used to split dataset
   * @param[in] range the range of these data points in the sub-dataset:
   * {the left index of the sub-dataset in the dataset, the size of the
   * sub-dataset}
   * @param[in] dataset the dataset needed to be divided, each element is the
   * pair: {key, value}
   * @param[out] subData the range of each sub-dataset after being split, each
   * element is: {the left index of each sub-dataset in the dataset, the size of
   * each sub-dataset}
   */
  template <typename InnerNodeType>
  void NodePartition(const InnerNodeType &node, const IndexPair &range,
                     const DataVectorType &dataset,
                     std::vector<IndexPair> *subData) const;

  /**
   * @brief Use this node to split the data points of the given dataset and
   * return the range of each sub-dataset. This dataset only includes the key
   * values.
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] node the current node used to split dataset
   * @param[in] range the range of these data points in the sub-dataset:
   * {the left index of the sub-dataset in the dataset, the size of the
   * sub-dataset}
   * @param[in] dataset the dataset needed to be divided, each element is the
   * key value
   * @param[out] subData the range of each sub-dataset after being split, each
   * element is: {the left index of each sub-dataset in the dataset, the size of
   * each sub-dataset}
   */
  template <typename InnerNodeType>
  void NodePartition(const InnerNodeType &node, const IndexPair &range,
                     const KeyVectorType &dataset,
                     std::vector<IndexPair> *subData) const;

  /**
   * @brief Construct a temporary inner node whose type is the given
   * InnerNodeType parameter, and use it to divide initDataset, findQuery
   * and insertQuery. Finally, return this node and the range of its child
   * nodes.
   *
   * @tparam InnerNodeType the type of this node
   * @param[in] range the range of these data points in the three
   * datasets: {initDataset: {left, size}, findQuery: {left, size}, insertQuery:
   * {left, size}}
   * @param[in] c the child number of this node
   * @param[out] subDataset the starting index and size of sub-dataset in each
   * child node, each element is: {the vector of the sub-initDataset, the vector
   * of the sub-findDataset, the vector of sub-insertDataset}. Each sub-dataset
   * is represented by: {left, size}, which means the range of it in the dataset
   * is [left, left + size).
   * @return InnerNodeType: node
   */
  template <typename InnerNodeType>
  InnerNodeType InnerDivideAll(const DataRange &range, int c,
                               SubDataset *subDataset);

  /**
   * @brief Update the previousLeaf and nextLeaf of each leaf node.
   */
  void UpdateLeaf();

 public:
  //*** Public Data Members of CARMI Objects

  /**
   * @brief Used to manage all nodes.
   */
  NodeArrayStructure<KeyType, ValueType, Compare, Alloc> node;

  /**
   * @brief Used to manage and store data points.
   */
  DataArrayStructure<KeyType, ValueType, Alloc> data;

  /**
   * @brief The pointer to the location of the external dataset.
   */
  const void *external_data;

  /**
   * @brief The length of a record of the external dataset, in bytes.
   */
  int recordLength;

  /**
   * @brief The index of the first leaf node.
   */
  int firstLeaf;

  /**
   * @brief The index of the last leaf node.
   */
  int lastLeaf;

  /**
   * @brief The current size of data points.
   */
  int currsize;

 private:
  //*** Private Data Members of CARMI Objects

  /**
   * @brief The root node.
   */
  CARMIRoot<DataVectorType, KeyType> root;

  /**
   * @brief The parameter of the cost model: cost = time + lambda * space
   */
  double lambda;

  /**
   * @brief Used to indicate whether the current CARMI is a primary index.
   */
  bool isPrimary;

 private:
  //*** Private Data Members of CARMI Objects for Construction

  /**
   * @brief The last index of the prefetched data block. This parameter is
   * useless after the index is constructed.
   */
  int prefetchEnd;

  /**
   * @brief The total frequency of queries. This parameter is useless after the
   * index is constructed.
   */
  int querySize;

  /**
   * @brief The space needed to be reserved for the future inserts. This
   * parameter is useless after the index is constructed.
   */
  int reservedSpace;

  /**
   * @brief Used to indicate whether the current mode is the init mode. The init
   * mode is that we store leaf nodes according to the prefetch prediction
   * model, and the other modes are that leaf nodes are not stored according to
   * the model. This parameter is useless after the index is constructed.
   */
  bool isInitMode;

  /**
   * @brief The initialized dataset. This parameter is useless after the index
   * is constructed. Each element is: {key, value}.
   */
  DataVectorType initDataset;

  /**
   * @brief The historical find queries. This parameter is useless after the
   * index is constructed. Each element is: {key value, the visited times of
   * this data point}.
   */
  QueryType findQuery;

  /**
   * @brief The historical insert queries. This parameter is useless after the
   * index is constructed. Each element is the key value.
   */
  KeyVectorType insertQuery;

  /**
   * @brief The cost of different sub-datasets, used for the memorized DP
   * algorithm. Each element is: {{the left index in the initDataset, the size
   * of the sub-dataset}, {the time cost, the space cost, the total cost}}. This
   * parameter is useless after the index is constructed.
   */
  std::map<IndexPair, NodeCost> COST;

  /**
   * @brief The optimal nodes corresponding to the sub-datasets. Each element
   * is: {{the left index in the initDataset, the size of the sub-dataset}, the
   * optimal node structure}. This parameter is useless after the index is
   * constructed.
   */
  std::map<IndexPair, BaseNode<KeyType, ValueType, Compare, Alloc>> structMap;

  /**
   * @brief The order of leaf nodes. This parameter is useless after the index
   * is constructed.
   */
  std::vector<int> scanLeaf;

  /**
   * @brief The index of leaf nodes which cannot be stored as the prefetch
   * prediction model. This parameter is useless after the index is constructed.
   */
  std::vector<int> remainingNode;

  /**
   * @brief The range of the sub-datasets whose nodes are leaf nodes and these
   * leaf nodes cannot be stored as the prefetch prediction model. This
   * parameter is useless after the index is constructed.
   */
  std::vector<DataRange> remainingRange;

  /**
   * @brief The empty leaf node. This parameter is useless after the index is
   * constructed.
   */
  BaseNode<KeyType, ValueType, Compare, Alloc> emptyNode;

 private:
  // *** Static Constant Options and Values of CARMI

  /**
   * @brief The empty range. This parameter is useless after the index is
   * constructed.
   */
  static constexpr IndexPair emptyRange = {-1, 0};

  /**
   * @brief The empty cost of space, time and total cost are all 0. This
   * parameter is useless after the index is constructed.
   */
  static constexpr NodeCost emptyCost = {0, 0, 0};

  /**
   * @brief The size of a node, in MB.
   */
  static constexpr double kBaseNodeSpace = 64.0 / 1024 / 1024;

  /**
   * @brief The space cost of the p. lr root node, in MB.
   */
  static constexpr double kPLRRootSpace =
      sizeof(PLRType<DataVectorType, KeyType>) / 1024.0 / 1024.0;

  /**
   * @brief The maximum child number in the histogram inner nodes.
   */
  static constexpr int kHisMaxChildNumber = 256;

  /**
   * @brief The maximum child number in the bs inner nodes.
   */
  static constexpr int kBSMaxChildNumber = 16;

  /**
   * @brief The minimum child number of inner nodes.
   */
  static constexpr int kMinChildNumber = 16;

  /**
   * @brief The number of new leaf nodes when splitting a leaf node.
   */
  static constexpr int kInsertNewChildNumber = 16;
};

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
CARMI<KeyType, ValueType, Compare, Alloc>::CARMI() {
  // set the default values to the variables
  isPrimary = false;
  firstLeaf = -1;
  lastLeaf = 0;
  isInitMode = true;
  prefetchEnd = -1;

  key_less_ = Compare();
  allocator_ = Alloc();

  emptyNode.cfArray = CFArrayType<KeyType, ValueType, Compare, Alloc>();
  reservedSpace = 0;
  data = DataArrayStructure<KeyType, ValueType, Alloc>(
      CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxBlockNum, 1000);
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
CARMI<KeyType, ValueType, Compare, Alloc>::CARMI(
    const DataVectorType &initData, const QueryType &findData,
    const KeyVectorType &insertData, double l) {
  if (carmi_params::kMaxLeafNodeSize <= 0 &&
      (carmi_params::kMaxLeafNodeSize % 64 != 0)) {
    throw std::logic_error(
        "carmi_params::kMaxLeafNodeSize does not meet the logic requirements.");
  }
  if (carmi_params::kAlgorithmThreshold < carmi_params::kMaxLeafNodeSize ||
      carmi_params::kAlgorithmThreshold <
          carmi_params::kMaxLeafNodeSizeExternal) {
    throw std::logic_error(
        "carmi_params::kAlgorithmThreshold does not meet the logic "
        "requirements.");
  }

  // set the default values to the variables
  isPrimary = false;
  lambda = l;
  firstLeaf = -1;
  lastLeaf = 0;
  isInitMode = true;
  prefetchEnd = -1;

  // generate initDataset, findQuery, insertQuery
  initDataset = std::move(initData);
  findQuery = std::move(findData);
  currsize = initDataset.size();
  if (findData.size() == 0) {
    // default to a read-only workload to construct the optimal index structure.
    findQuery.resize(initData.size());
    for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
      findQuery[i].first = initDataset[i].first;
      findQuery[i].second = 1;
    }
  }
  insertQuery = std::move(insertData);
  emptyNode.cfArray = CFArrayType<KeyType, ValueType, Compare, Alloc>();
  reservedSpace = insertQuery.size() * 1.0 / initDataset.size() * 4096 * 16;

  // calculate the total number of queries
  querySize = 0;
  for (int i = 0; i < static_cast<int>(findQuery.size()); i++) {
    querySize += findQuery[i].second;
  }
  querySize += insertQuery.size();

  data = DataArrayStructure<KeyType, ValueType, Alloc>(
      CFArrayType<KeyType, ValueType, Compare, Alloc>::kMaxBlockNum,
      initDataset.size());
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
CARMI<KeyType, ValueType, Compare, Alloc>::CARMI(
    const void *dataset, const KeyVectorType &insertData, double l,
    int record_number, int record_len) {
  if (carmi_params::kMaxLeafNodeSize <= 0 &&
      (carmi_params::kMaxLeafNodeSize % 64 != 0)) {
    throw std::logic_error(
        "carmi_params::kMaxLeafNodeSize does not meet the logic requirements.");
  }
  if (carmi_params::kAlgorithmThreshold < carmi_params::kMaxLeafNodeSize ||
      carmi_params::kAlgorithmThreshold <
          carmi_params::kMaxLeafNodeSizeExternal) {
    throw std::logic_error(
        "carmi_params::kAlgorithmThreshold does not meet the logic "
        "requirements.");
  }

  // set the default values to the variables
  recordLength = record_len;
  currsize = record_number;
  isInitMode = true;
  isPrimary = true;
  lambda = l;
  prefetchEnd = -1;

  // store the pointer to the dataset
  external_data = dataset;

  emptyNode.externalArray = ExternalArray<KeyType, ValueType, Compare>();

  initDataset.resize(record_number);
  findQuery.resize(record_number);
  insertQuery = std::move(insertData);

  querySize = 0;
  // generate initDataset, findQuery, insertQuery
  for (int i = 0; i < record_number; i++) {
    initDataset[i] = {*reinterpret_cast<const KeyType *>(
                          static_cast<const char *>(dataset) + i * record_len),
                      1};
    findQuery[i] = {initDataset[i].first, 1};
    // calculate the total number of queries
    querySize++;
  }
  std::sort(initDataset.begin(), initDataset.end(),
            [this](const DataType &a, const DataType &b) {
              return key_less_(a.first, b.first);
            });
  std::sort(findQuery.begin(), findQuery.end(),
            [this](const std::pair<KeyType, int> &a,
                   const std::pair<KeyType, int> &b) {
              return key_less_(a.first, b.first);
            });
  std::sort(
      insertQuery.begin(), insertQuery.end(),
      [this](const KeyType &a, const KeyType &b) { return key_less_(a, b); });

  querySize += insertQuery.size();

  reservedSpace =
      static_cast<float>(insertQuery.size()) / initDataset.size() * 4096 * 16;
}
#endif  // CARMI_H_
