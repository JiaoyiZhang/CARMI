/**
 * @file cfarray_type.h
 * @author Jiaoyi
 * @brief cache-friendly array leaf node
 * @version 3.0
 * @date 2021-03-11
 *
 * @copyright Copyright (c) 2021
 *
 */
#ifndef NODES_LEAFNODE_CFARRAY_TYPE_H_
#define NODES_LEAFNODE_CFARRAY_TYPE_H_

#include <float.h>
#include <math.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "../../construct/structures.h"
#include "../../memoryLayout/data_array.h"

/**
 * @brief The structure of cache-friendly array leaf node, which makes full use
 * of the cache/prefetch mechanism to speed up the data access.
 *
 * The data points are compactly stored in data blocks in a sequential manner in
 * the CF array leaf nodes. When searching for data points in the CF array leaf
 * nodes, we first search sequentially in the keywords of each block, and then
 * search within the block. When we need to insert a given data point, we must
 * first find the correct position, and then move all the data points by one
 * cell to make room for the new data point.
 *
 * @tparam KeyType the type of the key value
 * @tparam ValueType the type of the value
 * @tparam Compare A binary predicate that takes two element keys as arguments
 * and returns a bool.
 * @tparam Alloc Type of the allocator object used to define the storage
 * allocation model.
 */
template <typename KeyType, typename ValueType,
          typename Compare = std::less<KeyType>,
          typename Alloc = std::allocator<LeafSlots<KeyType, ValueType>>>
class CFArrayType {
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
   * @brief Construct a new CFArrayType object and initialize the members
   *
   * CFArrayType is a two-layer cache-friendly leaf node in CARMI, which manages
   * data points. The data points are compactly stored in data blocks in a
   * sequential manner in the second layer.
   *
   * When searching for data points in this node, we first search sequentially
   * in the slotkeys, and then search in a data block.
   */
  CFArrayType() {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
    m_left = -1;
    previousLeaf = -1;
    nextLeaf = -1;
    for (int i = 0; i < kMaxBlockNum - 1; i++) {
      slotkeys[i] = static_cast<KeyType>(DBL_MAX);
    }
    for (int i = 0; i < kMaxPerSizeNum; i++) {
      perSize[i] = 0;
    }
  }

 private:
  // *** Static Comparison Functions of the CF Array Leaf Node

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The second key value
   * @retval true a < b
   * @retval false a >= b
   */
  inline static bool key_less(const KeyType &a, const KeyType &b) {
    return key_less_(a, b);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first key value
   * @param[in] b The second key value
   * @retval true a <= b
   * @retval false a > b
   */
  inline static bool key_lessequal(const KeyType &a, const KeyType &b) {
    return !key_less_(b, a);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first key value
   * @param[in] b The second key value
   * @retval true a > b
   * @retval false a <= b
   */
  inline static bool key_greater(const KeyType &a, const KeyType &b) {
    return key_less_(b, a);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first key value
   * @param[in] b The second key value
   * @retval true a >= b
   * @retval false a < b
   */
  inline static bool key_greaterequal(const KeyType &a, const KeyType &b) {
    return !key_less_(a, b);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first key value
   * @param[in] b The second key value
   * @retval true a == b
   * @retval false a != b
   */
  inline static bool key_equal(const KeyType &a, const KeyType &b) {
    return !key_less_(a, b) && !key_less_(b, a);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The key value
   * @retval true a < b
   * @retval false a >= b
   */
  inline static bool data_less(const DataType &a, const KeyType &b) {
    return key_less_(a.first, b);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The key value
   * @retval true a <= b
   * @retval false a > b
   */
  inline static bool data_lessequal(const DataType &a, const KeyType &b) {
    return !key_less_(b, a.first);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The key value
   * @retval true a > b
   * @retval false a <= b
   */
  inline static bool data_greater(const DataType &a, const KeyType &b) {
    return key_less_(b, a.first);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The key value
   * @retval true a >= b
   * @retval false a < b
   */
  inline static bool data_greaterequal(const DataType &a, const KeyType &b) {
    return !key_less_(a.first, b);
  }

  /**
   * @brief Compare two elements.
   *
   * @param[in] a The first data point
   * @param[in] b The key value
   * @retval true a == b
   * @retval false a != b
   */
  inline static bool data_equal(const DataType &a, const KeyType &b) {
    return !key_less_(a.first, b) && !key_less_(b, a.first);
  }

 public:
  // *** Static Functions of the CF Array Leaf Node

  /**
   * @brief Get the actual size of data blocks
   *
   * @param[in] size the size of the data points
   * @return int the number of needed data blocks
   */
  static int CalNeededBlockNum(int size);

  /**
   * @brief extract data points (delete useless gaps and deleted data points)
   *
   * @param[in] data the data array
   * @param[in] blockleft the left index of data blocks managed by the leaf node
   * @param[in] blockright the right index of data blocks managed by the leaf
   * node
   * @return DataVectorType : pure data points stored in the vector
   */
  static DataVectorType ExtractDataset(
      const DataArrayStructure<KeyType, ValueType, Alloc> &data, int blockleft,
      int blockright);

 public:
  // *** Basic Functions of CF Array Leaf Objects

  /**
   * @brief initialize cf array node and store the given dataset into data
   * blocks
   *
   * @param[in] dataset the dataset
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] start_idx the starting index of data points, the size of data
   * points is equal to the size of the prefetchIndex vector
   * @param[inout] data the data array
   */
  void Init(const DataVectorType &dataset,
            const std::vector<int> &prefetchIndex, int start_idx,
            DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief Find the corresponding data point of the given key value in the data
   * array and return the index of the data block and the index of the data
   * point in this data block. If unsuccessful, return -1.
   *
   * @param[in] data the data array
   * @param[in] key the given key value
   * @param[out] currblock the index of the data block
   * @return int: the index of the data point in this data block
   * @retval kMaxBlockCapacity: find the data point unsuccessfully
   */
  inline int Find(const DataArrayStructure<KeyType, ValueType, Alloc> &data,
                  const KeyType &key, int *currblock) const;

  /**
   * @brief Insert the datapoint into this leaf node
   *
   * The basic process of the insert operation is similar to the lookup
   * operation. After finding the correct data block for insertion, we insert
   * the data point into it. In addition, there are two mechanisms that can be
   * initiated by the leaf node under certain situations: expand and rebalance
   * mechanisms.
   *
   * @param[in] datapoint the inserted data point
   * @param[out] currblock the index of the data block
   * @param[out] currslot the index of the data point in the data block in
   * the cf array leaf node
   * @param[inout] data the data array after the insert
   * @retval true the insert is successful
   * @retval false the insert fails
   */
  inline bool Insert(const DataType &datapoint, int *currblock, int *currslot,
                     DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief Delete all the records of the given key value
   *
   * The basic process of the delete operation is similar to the update
   * operation. After finding the correct data block, we find all the data
   * points regarding the given key value and delete them. If there is no
   * corresponding data point, return true directly.
   *
   * @param[in] key the given key value
   * @param[out] cnt the number of deleted data points
   * @param[inout] data the data array after the delete
   * @retval true the delete is successful
   */
  inline bool Delete(const KeyType &key, size_t *cnt,
                     DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief Delete the data point at the given position
   *
   * @param[in] key the given key value
   * @param[in] currblock the index of the data block of the leaf node
   * @param[in] currslot the index of the data point in the data block in
   * the cf array leaf node
   * @param[inout] data the data array after the delete
   * @retval true if the operation succeeds
   * @retval false if the operation fails (the given position is invalid)
   */
  inline bool DeleteSingleData(
      const KeyType &key, int currblock, int currslot,
      DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief store data points into the data array
   *
   * @param[in] dataset the dataset
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] isInitMode the current construction mode. If this parameter is
   * true, we should store the data points according to the location given by
   * the predict prefetching model as far as possible. Otherwise, we should
   * store them evenly in these data blocks.
   * @param[in] neededBlockNum the number of needed data blocks
   * @param[in] left the left index of these data points in the dataset
   * @param[in] size the size of these data points in the dataset
   * @param[inout] data the data array
   * @param[inout] prefetchEnd the right index of prefetched blocks in initial
   * mode, this parameter is useless if the isInitMode is false.
   * @retval true store data points in the data array successfully
   * @retval false store data points unsuccessfully, this status will not occur.
   */
  inline bool StoreData(const DataVectorType &dataset,
                        const std::vector<int> &prefetchIndex, bool isInitMode,
                        int neededBlockNum, int left, int size,
                        DataArrayStructure<KeyType, ValueType, Alloc> *data,
                        int *prefetchEnd);

 public:
  //*** Sub-Functions of Find

  /**
   * @brief Get the size of the data point
   *
   * @param[in] currblock the index of the data block
   * @return int the size of the data block
   */
  inline int GetBlockSize(int currblock) { return perSize[currblock]; }

  /**
   * @brief Get the actual number of data points in data.dataArray[blockleft,
   * blockright)
   *
   * @return int: the number of data points
   */
  inline int GetDataNum();

  /**
   * @brief Compare the given key value with slotkeys to search the index of the
   * corresponding data block of the given key value. This function is the
   * sub-function of find function, and we set it as public due to the need
   * to test the CPU time.
   *
   * @param[in] key the given key value
   * @return int: the index of the data block
   */
  inline int Search(const KeyType &key) const;

  /**
   * @brief Search the data point of the given key value in the given data block
   * and return its index. If unsuccessfully, return kMaxBlockCapacity.This
   * function is the sub-function of the find function, and we set it as public
   * due to the need to test the CPU time.
   *
   * @param[in] block the current data block
   * @param[in] key the given key value
   * @param[in] currsize the size of the current data block
   * @return int: the index of the data point in this data block
   * @retval kMaxBlockCapacity: if we fail to find the corresponding data
   * point, return kMaxBlockCapacity
   */
  inline int SearchDataBlock(const LeafSlots<KeyType, ValueType> &block,
                             const KeyType &key, int currsize) const;

 private:
  // *** Private functions for insert operations

  /**
   * @brief If the data block to be inserted is full and the next data block
   * is also saturated, the rebalance mechanism is triggered. The leaf node
   * collects all data points in all managed data blocks and reallocates them
   * evenly to all these blocks.
   *
   * @param[in] blockleft the left index of data blocks
   * @param[in] blockright the right index of data blocks
   * @param[inout] data the data array
   */
  inline void Rebalance(int blockleft, int blockright,
                        DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief When this leaf node needs more space to store more data points,
   * the expand operation can be initiated to get more data blocks. We first
   * collect all the data points managed by this leaf node, and then get a new
   * location with more space in the data array.
   *
   * @param[in] blockleft the left index of data blocks
   * @param[in] blockright the right index of data blocks
   * @param[inout] data the data array
   */
  inline void Expand(int blockleft, int blockright,
                     DataArrayStructure<KeyType, ValueType, Alloc> *data);

  /**
   * @brief insert the given data point into the data block
   *
   * @param[in] datapoint the inserted data point
   * @param[in] currBlockIdx the index of the current data blocks in this node
   * @param[inout] currBlock the data block into which the data point will be
   * inserted
   * @retval true this operation is successful and the second value is the
   * position of the inserted data point
   * @retval false if this data block is full, return false and the second value
   * is useless
   */
  std::pair<bool, int> InsertDataBlock(
      const DataType &datapoint, int currBlockIdx,
      LeafSlots<KeyType, ValueType> *currBlock);

  /**
   * @brief insert a data point into the next data block in the leaf node
   *
   * @param[in] currdata the data points needed to be inserted
   * @param[inout] currBlockIdx the index of the current data block in the leaf
   * node
   * @param[inout] data the data array
   * @retval true this operation is successful and the second value is the
   * position of the inserted data point
   * @retval false if this data block is full, return false and the second value
   * is useless
   */
  inline std::pair<bool, int> InsertNextBlock(
      const DataType &currdata, int *currBlockIdx,
      DataArrayStructure<KeyType, ValueType, Alloc> *data);

 private:
  //*** Private Sub-Functions of StoreData

  /**
   * @brief check whether these data points can be stored as prefetched
   *
   * @param[in] data the data array
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] neededBlockNum the needed number of data blocks
   * @retval true these data points can be stored as prefetched
   * @retval false these data points cannot be stored as prefetched
   */
  bool CheckIsPrefetch(
      const DataArrayStructure<KeyType, ValueType, Alloc> &data,
      const std::vector<int> &prefetchIndex, int neededBlockNum);

  /**
   * @brief store data points in such a way that extra data points are filled
   * with the previous blocks first
   *
   * @param[in] dataset the dataset to be stored
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] neededBlockNum the number of needed data blocks to store all the
   * data points
   * @param[out] tmpBlockVec data blocks to store data points
   * @param[out] actualBlockNum the actual number of used data blocks
   * @param[out] missNumber the number of data points which are not stored as
   * the prefetching index
   * @retval true these data points have been stored as prefetched
   * @retval false the given data blocks cannot accomodate these data points
   */
  inline bool StorePrevious(
      const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
      int neededBlockNum,
      std::vector<LeafSlots<KeyType, ValueType>> *tmpBlockVec,
      int *actualBlockNum, int *missNumber);

  /**
   * @brief store data points in such a way that extra data points are filled
   * with the subsequent blocks first
   *
   * @param[in] dataset the dataset to be stored
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] neededBlockNum the number of needed data blocks to store all the
   * data points
   * @param[out] tmpBlockVec blocks to store data points
   * @param[out] actualBlockNum the actual number of used data blocks
   * @param[out] missNumber the number of data points which are not stored as
   * the prefetching index
   * @retval true these data points have been stored as prefetched
   * @retval false the given data blocks cannot accomodate these data points
   */
  inline bool StoreSubsequent(
      const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
      int neededBlockNum,
      std::vector<LeafSlots<KeyType, ValueType>> *tmpBlockVec,
      int *actualBlockNum, int *missNumber);

 public:
  // *** Static Constant Options and Values of the CF Array Leaf Node

  /**
   * @brief CF array leaf node parameter: The maximum number of data points in
   * each data block, which depends on the carmi_params::kMaxLeafNodeSize and
   * the type of data points.
   */
  static constexpr int kMaxBlockCapacity =
      carmi_params::kMaxLeafNodeSize / sizeof(DataType);

  /** @brief CF array leaf node parameter: The maximum number of data blocks in
   * each cf array leaf node, which depends on the type of the key value.
   */
  static constexpr int kMaxBlockNum =
      static_cast<int>(48 / (sizeof(KeyType) + 1)) + 1;

  /**
   * @brief CF array leaf node parameter: The maximum amount of  capacity per
   * data block in each cf array leaf node. Each element is the number of data
   * points stored in each data block. The maximum number of data points in each
   * data block is 256.
   */
  static constexpr int kMaxPerSizeNum =
      48 - (kMaxBlockNum - 1) * sizeof(KeyType);

  /**
   * @brief CF array leaf node parameter: The maximum number of data points in
   * each cf array leaf node, which is the result of kMaxBlockCapacity *
   * kMaxBlockNum.
   */
  static constexpr int kMaxLeafCapacity = kMaxBlockCapacity * kMaxBlockNum;

  /**
   * @brief Compare two elements.
   */
  static constexpr Compare key_less_ = Compare();

 public:
  //*** Public Data Members of CF Array Leaf Node Objects

  /**
   * @brief  A combined integer, composed of the flag of CF array leaf node and
   * the number of data blocks managed by it. (4 bytes)
   */
  int flagNumber;

  /**
   * @brief The first index of data blocks managed by this object in the data
   * array. (4 bytes)
   */
  int m_left;

  /**
   * @brief The index of the previous leaf node in the data array, used for
   * range scan workloads. (4 bytes)
   */
  int previousLeaf;

  /**
   * @brief The index of the next leaf node in the data array, used for range
   * scan workloads. (4 bytes)
   */
  int nextLeaf;

  /**
   * @brief An array of length kMaxBlockNum-1, used to store the minimum key
   * value in each data block managed by this object except the first data
   * block. When searching for a data point with a given key value, first find
   * the corresponding data block through sequentially searching in this array.
   * (48 bytes)
   */
  KeyType slotkeys[kMaxBlockNum - 1];

  /**
   * @brief An array of length kMaxPerSizeNum, used to store each capacity of
   * the data block. The upper limit of the size of each data block is 256.
   */
  char perSize[kMaxPerSizeNum];
};

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline int CFArrayType<KeyType, ValueType, Compare, Alloc>::CalNeededBlockNum(
    int size) {
  if (size <= 0) return 0;
#ifdef CHECK
  if (size > kMaxLeafCapacity) {
    std::cout << "the size is " << size
              << ",\tthe maximum size in a leaf node is " << kMaxLeafCapacity
              << std::endl;
  }
#endif  // CHECK
  int neededBlockNumber = std::min(
      static_cast<int>(ceil(size * 1.0 / kMaxBlockCapacity)), kMaxBlockNum);

  return neededBlockNumber;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
int CFArrayType<KeyType, ValueType, Compare, Alloc>::GetDataNum() {
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  int cnt = 0;
  for (int i = 0; i < nowBlockNum; i++) {
    cnt += perSize[i];
  }
  return cnt;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
typename CFArrayType<KeyType, ValueType, Compare, Alloc>::DataVectorType
CFArrayType<KeyType, ValueType, Compare, Alloc>::ExtractDataset(
    const DataArrayStructure<KeyType, ValueType, Alloc> &data, int blockleft,
    int blockright) {
  if (blockleft < 0 || blockleft > blockright ||
      blockright > data.dataArray.size()) {
    throw std::out_of_range(
        "CFArrayType::ExtractDataset: the range is invalid.");
  }

  DataVectorType currdata;
  for (int i = blockleft; i < blockright; i++) {
    for (int j = 0; j < kMaxBlockCapacity; j++) {
      if (data.dataArray[i].slots[j].first != static_cast<KeyType>(DBL_MAX)) {
        currdata.push_back(data.dataArray[i].slots[j]);
      }
    }
  }
  return currdata;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline void CFArrayType<KeyType, ValueType, Compare, Alloc>::Init(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int start_idx, DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  int size = prefetchIndex.size();
  if (size == 0) {
    return;
  }
  if (start_idx < 0 || start_idx >= data->dataArray.size()) {
    throw std::out_of_range("CFArrayType::Init: the range is invalid.");
  }

  int neededBlockNum = CalNeededBlockNum(size);
  StoreData(dataset, prefetchIndex, false, neededBlockNum, start_idx, size,
            data, 0);
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline int CFArrayType<KeyType, ValueType, Compare, Alloc>::Find(
    const DataArrayStructure<KeyType, ValueType, Alloc> &data,
    const KeyType &key, int *currblock) const {
  // Use Search function to find the data block which stores the data point of
  // the given key value.
  // currblock is the serial number of this data block in all data blocks
  // managed by this leaf node.
  *currblock = Search(key);

  // Search the data point in this data block
  int res = SearchDataBlock(data.dataArray[m_left + *currblock], key,
                            perSize[*currblock]);

  // Return the index of the data point. If res is equal to kMaxBlockCapacity,
  // finding the corresponding data point fails.
  return res;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::Insert(
    const DataType &datapoint, int *currblock, int *currslot,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  int nowBlockNum = flagNumber & 0x00FFFFFF;

  // Case 1: this node is empty.
  // If this node is empty, we first need to allocate a block of memory for it
  // and use the first index to update m_left, then insert the data point in the
  // first allocated data block. After that, we update the number of data blocks
  // managed by this node and return directly.
  if (nowBlockNum == 0) {
    m_left = data->AllocateMemory(1);
    data->dataArray[m_left].slots[0] = datapoint;
    slotkeys[0] = datapoint.first + 1;
    perSize[0] = 1;
    *currblock = 0;
    flagNumber++;
    return true;
  }

  // Case 2: insert into the current data block.
  // Use the Search function to find the corresponding data block to store this
  // data point. currblock is the serial number of this data block in all data
  // blocks managed by this leaf node. Then insert the data point into this data
  // block, return directly if successful.
  *currblock = Search(datapoint.first);
  auto tmpRes = InsertDataBlock(datapoint, *currblock,
                                &(data->dataArray[m_left + *currblock]));
  bool isSuccess = tmpRes.first;
  if (isSuccess) {
    *currslot = tmpRes.second;
    return true;
  }

  int nowDataNum = GetDataNum();

  // Case 3: insert into the next data block.
  // If inserting into the current data block fails, and there is an empty
  // position in the next data block, insert into the next data block.
  // Return directly if successful.
  if (*currblock < nowBlockNum - 1 &&
      perSize[*currblock + 1] < kMaxBlockCapacity) {
    tmpRes = InsertNextBlock(datapoint, currblock, data);
    isSuccess = tmpRes.first;
    if (isSuccess) {
      *currslot = tmpRes.second;
      return true;
    }
  }

  // Case 4: If the latter data block has no empty position, we first need some
  // additional mechanisms to make room for the inserted data point.
  if (nowDataNum < nowBlockNum * (kMaxBlockCapacity - 1)) {
    // Case 4.1: rebalance.
    // If the data block to be inserted is full and the next data block is also
    // saturated, the rebalance mechanism is triggered. Through this mechnism,
    // we can reallocate the data points evenly to all the data blocks.
    Rebalance(m_left, m_left + nowBlockNum, data);
  } else if (nowBlockNum == kMaxBlockNum) {
    // Case 4.2: return false.
    // If the data blocks have reached the upper limit, this node cannot expand.
    // Return false directly.
    return false;
  } else {
    // Case 4.3: expand.
    // This leaf node needs more space to store data points, we can initiate
    // the expand operation to get more data blocks.
    Expand(m_left, m_left + nowBlockNum, data);
  }
  // After the corresponding mechnism, there are some empty positions for the
  // inserted data point.
  *currblock = Search(datapoint.first);
  tmpRes = InsertDataBlock(datapoint, *currblock,
                           &(data->dataArray[m_left + *currblock]));
  isSuccess = tmpRes.first;
  *currslot = tmpRes.second;
  return isSuccess;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::Delete(
    const KeyType &key, size_t *cnt,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  // Get the number of data blocks
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  // Find the corresponding data block which stores this data point.
  int currBlockIdx = m_left + Search(key);
  // Search the data point in this data block
  int res = SearchDataBlock(data->dataArray[currBlockIdx], key,
                            perSize[currBlockIdx - m_left]);

  // Delete all the data points related to the given key value.
  // deletedCnt is the number of all deleted data points.
  int deletedCnt = 0;
  for (int i = currBlockIdx, j = res;
       i < m_left + nowBlockNum && j < perSize[i - m_left]; j++) {
    if (data->dataArray[i].slots[j].first == key) {
      // Case 2: this slot stores the data point to be deleted.
      // Clear this data point first and updated the delete counter. If this
      // slot is the last one of this data block, continue to check the next
      // data block.
      data->dataArray[i].slots[j] = {static_cast<KeyType>(DBL_MAX),
                                     static_cast<KeyType>(DBL_MAX)};
      perSize[i - m_left]--;
      deletedCnt++;
      if (j == kMaxBlockCapacity - 1) {
        j = -1;
        i++;
      }
    } else {
      // Case 3: this slot stores other data points.
      // This indicates that all data points to be deleted have been cleared,
      // and the subsequent data points need to be moved to remove their empty
      // slots.
      if (deletedCnt == 0) {
        break;
      }
      for (; j < kMaxBlockCapacity; j++) {
        data->dataArray[currBlockIdx].slots[j - deletedCnt] =
            data->dataArray[currBlockIdx].slots[j];
      }
      break;
    }
  }
  *cnt = deletedCnt;
  return true;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::DeleteSingleData(
    const KeyType &key, int currblock, int currslot,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  // Get the number of data blocks
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  int blockIdx = m_left + currblock;

  // Case 1: the given position is invalid
  if (currblock < 0 || currblock >= nowBlockNum || currslot < 0 ||
      currslot >= kMaxBlockCapacity ||
      data->dataArray[blockIdx].slots[currslot].first != key) {
    return false;
  }

  // Case 2: delete the data point
  perSize[currblock]--;
  data->dataArray[blockIdx].slots[currslot] = {static_cast<KeyType>(DBL_MAX),
                                               static_cast<KeyType>(DBL_MAX)};
  currslot++;
  while (currslot < perSize[currblock]) {
    data->dataArray[blockIdx].slots[currslot - 1] =
        data->dataArray[blockIdx].slots[currslot];
  }
  return true;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline int CFArrayType<KeyType, ValueType, Compare, Alloc>::Search(
    const KeyType &key) const {
#if defined(CATCH_PLATFORM_LINUX) || defined(CATCH_PLATFORM_WINDOWS)
  return std::lower_bound(slotkeys, slotkeys + (flagNumber & 0x00FFFFFF) - 1,
                          key, key_lessequal) -
         slotkeys;
#elif defined(CATCH_PLATFORM_MAC)
  int end_idx = (flagNumber & 0x00FFFFFF) - 1;
  for (int i = 0; i < end_idx; i++) {
    if (key < slotkeys[i]) {
      return i;
    }
  }
  return end_idx;
#endif
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline int CFArrayType<KeyType, ValueType, Compare, Alloc>::SearchDataBlock(
    const LeafSlots<KeyType, ValueType> &block, const KeyType &key,
    int currsize) const {
#if defined(CATCH_PLATFORM_LINUX) || defined(CATCH_PLATFORM_WINDOWS)
  return std::lower_bound(block.slots, block.slots + currsize, key, data_less) -
         block.slots;
#elif defined(CATCH_PLATFORM_MAC)
  int i = 0;
  if (key <= block.slots[currsize / 2 - 1].first) {
    for (; i < currsize / 2; i++) {
      if (key <= block.slots[i].first) {
        break;
      }
    }
  } else {
    for (i = currsize / 2; i < currsize; i++) {
      if (key <= block.slots[i].first) {
        break;
      }
    }
  }
  return i;
#endif
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline void CFArrayType<KeyType, ValueType, Compare, Alloc>::Rebalance(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  // extract pure data points
  DataVectorType newDataset = ExtractDataset(*data, blockleft, blockright);
  int nowBlockNum = blockright - blockleft;
  std::vector<int> prefetchIndex(newDataset.size(), 0);

  // store data points
  StoreData(newDataset, prefetchIndex, false, nowBlockNum, 0, newDataset.size(),
            data, 0);
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline void CFArrayType<KeyType, ValueType, Compare, Alloc>::Expand(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  // extract pure data points
  DataVectorType newDataset = ExtractDataset(*data, blockleft, blockright);
  int neededBlockNum = blockright - blockleft + 1;
  std::vector<int> prefetchIndex(newDataset.size(), 0);

  // store data points
  StoreData(newDataset, prefetchIndex, false, neededBlockNum, 0,
            newDataset.size(), data, 0);
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
std::pair<bool, int>
CFArrayType<KeyType, ValueType, Compare, Alloc>::InsertDataBlock(
    const DataType &currdata, int currBlockIdx,
    LeafSlots<KeyType, ValueType> *currBlock) {
  // Case 1: this data block is full, return false directly
  int currsize = perSize[currBlockIdx];
  if (currsize >= kMaxBlockCapacity) {
    return {false, 0};
  }
  // Case 2: this first slot in the currBlock is empty, insert the data point
  // into it and update the slotkeys, then return true directly
  if (currsize == 0) {
    currBlock->slots[0] = currdata;
    perSize[currBlockIdx]++;
    if (currBlockIdx == 0) {
      slotkeys[0] = currdata.first + 1;
    } else {
      slotkeys[currBlockIdx - 1] = currdata.first;
    }
    return {true, 0};
  }
  // Case 3: insert the data point into other slots
  // first find the position to be inserted
  int res = SearchDataBlock(*currBlock, currdata.first, currsize);
  // Case 3.1: this slot is empty, insert it directly
  if (res >= currsize) {
    currBlock->slots[res] = currdata;
    perSize[currBlockIdx]++;
    if (currBlockIdx != 0) {
      slotkeys[currBlockIdx - 1] =
          std::min(slotkeys[currBlockIdx - 1], currdata.first);
    } else {
      if (currdata.first > slotkeys[0]) {
        slotkeys[0] = currdata.first + 1;
      }
    }
    return {true, res};
  }
  // Case 3.2: this slot is not empty
  // We first need to make room for the inserted data point
  // move data points after the inserted slot
  for (int num = currsize; num > res; num--) {
    currBlock->slots[num] = currBlock->slots[num - 1];
  }
  // store the data point into the slot
  currBlock->slots[res] = currdata;
  perSize[currBlockIdx]++;
  if (currBlockIdx != 0) {
    slotkeys[currBlockIdx - 1] =
        std::min(slotkeys[currBlockIdx - 1], currdata.first);
  } else {
    if (currdata.first > slotkeys[0]) {
      slotkeys[0] = currdata.first + 1;
    }
  }
  return {true, res};
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline std::pair<bool, int>
CFArrayType<KeyType, ValueType, Compare, Alloc>::InsertNextBlock(
    const DataType &currdata, int *currBlockIdx,
    DataArrayStructure<KeyType, ValueType, Alloc> *data) {
  int nowDataIdx = m_left + *currBlockIdx;
  int currslot = 0;
  // Case 1: the key value of the inserted data point is larger than the last
  // data point in the current data block, insert the given data point into
  // the next data block
  int tmpBlockIdx = *currBlockIdx;
  if (currdata.first >=
      data->dataArray[nowDataIdx].slots[kMaxBlockCapacity - 1].first) {
    auto res = InsertDataBlock(currdata, *currBlockIdx + 1,
                               &data->dataArray[nowDataIdx + 1]);
    currslot = res.second;
    tmpBlockIdx++;
  } else {
    // Case 2: the key value of the inserted data point is smaller than the
    // last data point in the current data block, insert the last data point
    // in the current data block into the next data block
    InsertDataBlock(data->dataArray[nowDataIdx].slots[kMaxBlockCapacity - 1],
                    *currBlockIdx + 1, &data->dataArray[nowDataIdx + 1]);

    // move data points and store the given data point in this data block
    for (int i = kMaxBlockCapacity - 1; i >= 0; i--) {
      if (i == 0) {
        data->dataArray[nowDataIdx].slots[0] = currdata;
        currslot = 0;
        if (*currBlockIdx != 0)
          slotkeys[*currBlockIdx - 1] =
              data->dataArray[nowDataIdx].slots[0].first;
        break;
      }
      if (data->dataArray[nowDataIdx].slots[i - 1].first > currdata.first) {
        data->dataArray[nowDataIdx].slots[i] =
            data->dataArray[nowDataIdx].slots[i - 1];
      } else {
        data->dataArray[nowDataIdx].slots[i] = currdata;
        currslot = i;
        break;
      }
    }
  }
  // update the slotkeys
  slotkeys[*currBlockIdx] = data->dataArray[nowDataIdx + 1].slots[0].first;
  *currBlockIdx = tmpBlockIdx;
  return {true, currslot};
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::CheckIsPrefetch(
    const DataArrayStructure<KeyType, ValueType, Alloc> &data,
    const std::vector<int> &prefetchIndex, int neededBlockNum) {
  // check whether these points can be prefetched
  int leftIdx = prefetchIndex[0];
  // Case 1: the needed data blocks exceed the exsiting data array
  // return false directly
  if (leftIdx + neededBlockNum > data.dataArray.size()) {
    return false;
  }
  int rightIdx = prefetchIndex[prefetchIndex.size() - 1];
  rightIdx = std::min(leftIdx + neededBlockNum - 1, rightIdx);
  // Case 2: if the predicted data blocks have been used before, return false
  for (int i = leftIdx; i <= rightIdx; i++) {
    if (data.dataArray[i].slots[0].first != static_cast<KeyType>(DBL_MAX)) {
      return false;
    }
  }
  // Case 3: these data points can be prefetched
  return true;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::StorePrevious(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int neededBlockNum, std::vector<LeafSlots<KeyType, ValueType>> *tmpBlockVec,
    int *actualBlockNum, int *missNumber) {
  int size = dataset.size();
  if (size > neededBlockNum * kMaxBlockCapacity) {
    return false;
  }
  std::map<KeyType, int> actualIdx;
  *actualBlockNum = 0;
  int leftIdx = prefetchIndex[0];
  for (int i = 0; i < size; i++) {
    int p = prefetchIndex[i];
    p -= leftIdx;
    if (p >= neededBlockNum) {
      p = neededBlockNum - 1;
    }
    auto tmpRes = InsertDataBlock(dataset[i], p, &(*tmpBlockVec)[p]);
    bool insertSuccess = tmpRes.first;
    DataType nowDatapoint = dataset[i];
    DataType preDatapoint;
    std::vector<LeafSlots<KeyType, ValueType>> tmp = *tmpBlockVec;
    int tmpP = p;

    // insert into the previous block
    while (!insertSuccess) {
      preDatapoint = (*tmpBlockVec)[p].slots[0];
      for (int j = 0; j < kMaxBlockCapacity - 1; j++) {
        (*tmpBlockVec)[p].slots[j] = (*tmpBlockVec)[p].slots[j + 1];
      }
      (*tmpBlockVec)[p].slots[kMaxBlockCapacity - 1] = nowDatapoint;
      p--;
      if (p < 0) {
        break;
      }
      tmpRes = InsertDataBlock(preDatapoint, p, &(*tmpBlockVec)[p]);
      insertSuccess = tmpRes.first;
      nowDatapoint = preDatapoint;
    }
    // insert into the subsequent block
    if (!insertSuccess) {
      p = tmpP;
      while (p + 1 < neededBlockNum && !insertSuccess) {
        *tmpBlockVec = tmp;
        p++;
        tmpRes = InsertDataBlock(dataset[i], p, &(*tmpBlockVec)[p]);
        insertSuccess = tmpRes.first;
      }
    }
    if (insertSuccess) {
      if (p + 1 > *actualBlockNum) {
        *actualBlockNum = p + 1;
      }
    } else {
      // the given data blocks cannot accomodate these data points
      return false;
    }
  }

  // count the number of data points which are not stored as the prefetching
  // index
  int tmpMissNum = 0;
  int cnt = 0;
  for (int i = 0; i < *actualBlockNum; i++) {
    for (int j = 0; j < kMaxBlockCapacity; j++) {
      if ((*tmpBlockVec)[i].slots[j].first == static_cast<KeyType>(DBL_MAX)) {
        break;
      }
      int fetchIdx = prefetchIndex[cnt++];
      fetchIdx -= leftIdx;
      if (fetchIdx != i) {
        tmpMissNum++;
      }
    }
  }
  *missNumber = tmpMissNum;

  return true;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::StoreSubsequent(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int neededBlockNum, std::vector<LeafSlots<KeyType, ValueType>> *tmpBlockVec,
    int *actualBlockNum, int *missNumber) {
  int size = dataset.size();
  if (size > neededBlockNum * kMaxBlockCapacity) {
    return false;
  }
  std::map<KeyType, int> actualIdx;
  *actualBlockNum = 0;
  int leftIdx = prefetchIndex[0];
  for (int i = 0; i < size; i++) {
    int p = prefetchIndex[i];
    p -= leftIdx;
    if (p >= neededBlockNum) {
      p = neededBlockNum - 1;
    }
    auto tmpRes = InsertDataBlock(dataset[i], p, &(*tmpBlockVec)[p]);
    bool insertSuccess = tmpRes.first;
    DataType nowDatapoint = dataset[i];
    DataType preDatapoint;
    // insert into the subsequent block
    while (!insertSuccess && p + 1 < neededBlockNum) {
      p++;
      tmpRes = InsertDataBlock(dataset[i], p, &(*tmpBlockVec)[p]);
      insertSuccess = tmpRes.first;
    }

    // insert into the previous block
    while (!insertSuccess) {
      preDatapoint = (*tmpBlockVec)[p].slots[0];
      for (int j = 0; j < kMaxBlockCapacity - 1; j++) {
        (*tmpBlockVec)[p].slots[j] = (*tmpBlockVec)[p].slots[j + 1];
      }
      (*tmpBlockVec)[p].slots[kMaxBlockCapacity - 1] = nowDatapoint;
      p--;
      if (p < 0) {
        break;
      }
      tmpRes = InsertDataBlock(preDatapoint, p, &(*tmpBlockVec)[p]);
      insertSuccess = tmpRes.first;
      nowDatapoint = preDatapoint;
    }

    if (insertSuccess) {
      if (p + 1 > *actualBlockNum) {
        *actualBlockNum = p + 1;
      }
    } else {
      return false;
    }
  }

  // count the number of data points which are not stored as the prefetching
  // index
  int tmpMissNum = 0;
  int cnt = 0;
  for (int i = 0; i < *actualBlockNum; i++) {
    for (int j = 0; j < kMaxBlockCapacity; j++) {
      if ((*tmpBlockVec)[i].slots[j].first == static_cast<KeyType>(DBL_MAX)) {
        break;
      }
      int fetchIdx = prefetchIndex[cnt++];
      fetchIdx -= leftIdx;
      if (fetchIdx != i) {
        tmpMissNum++;
      }
    }
  }

  *missNumber = tmpMissNum;
  return true;
}

template <typename KeyType, typename ValueType, typename Compare,
          typename Alloc>
inline bool CFArrayType<KeyType, ValueType, Compare, Alloc>::StoreData(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    bool isInitMode, int neededBlockNum, int left, int actualSize,
    DataArrayStructure<KeyType, ValueType, Alloc> *data, int *prefetchEnd) {
  // if the dataset is empty, return true directly
  for (int i = 0; i < kMaxPerSizeNum; i++) {
    perSize[i] = 0;
  }
  int size = prefetchIndex.size();
  if (neededBlockNum == 0 || size == 0) {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
    return true;
  }
  int end = left + size;
  int actualBlockNum = 0;
  bool isPossible = true;
  int leftIdx = prefetchIndex[0];
  if (isInitMode) {
    // check whether these points can be prefetched
    isPossible = CheckIsPrefetch(*data, prefetchIndex, neededBlockNum);
  }
  if (isInitMode && isPossible) {
    // if the current status is init mode and these data points can be stored
    // as prefetched, we then store them as the position in the prefetchIndex
    m_left = leftIdx;

    LeafSlots<KeyType, ValueType> tmpSlot;
    std::vector<LeafSlots<KeyType, ValueType>> prevBlocks(neededBlockNum,
                                                          tmpSlot);
    // store in a way that gives priority to the previous data blocks
    int prevActualNum = 0;
    int prevMissNum = 0;
    bool isPreviousSuccess =
        StorePrevious(dataset, prefetchIndex, neededBlockNum, &prevBlocks,
                      &prevActualNum, &prevMissNum);
    char previousPerSize[kMaxPerSizeNum];
    for (int i = 0; i < kMaxPerSizeNum; i++) {
      previousPerSize[i] = perSize[i];
      perSize[i] = 0;
    }

    std::vector<LeafSlots<KeyType, ValueType>> nextBlocks(neededBlockNum,
                                                          tmpSlot);
    // store in a way that gives priority to the subsequent data blocks
    int nextActualNum = 0;
    int nextMissNum = 0;
    bool isNextSuccess =
        StoreSubsequent(dataset, prefetchIndex, neededBlockNum, &nextBlocks,
                        &nextActualNum, &nextMissNum);

    bool isPrev = true;
    if (isNextSuccess && isPreviousSuccess) {
      if (nextMissNum < prevMissNum) {
        isPrev = false;
      }
    } else if (isNextSuccess) {
      isPrev = false;
    } else if (isPreviousSuccess) {
      isPrev = true;
    } else {
      // if the given data blocks cannot accomodate these data points, return
      // false directly
      return false;
    }
    // store data points into the data array
    if (isPrev) {
      actualBlockNum = prevActualNum;
      for (int i = m_left; i < m_left + actualBlockNum; i++) {
        data->dataArray[i] = prevBlocks[i - m_left];
      }
      for (int i = 0; i < kMaxPerSizeNum; i++) {
        perSize[i] = previousPerSize[i];
      }
    } else {
      actualBlockNum = nextActualNum;
      for (int i = m_left; i < m_left + actualBlockNum; i++) {
        data->dataArray[i] = nextBlocks[i - m_left];
      }
    }
    // update the usedDatasize in the data structure
    if (m_left + actualBlockNum > static_cast<int>(data->usedDatasize)) {
      data->usedDatasize = m_left + actualBlockNum;
    }
    // add the gap between the prefetchEnd and m_left into emptyBlocks
    if (m_left - *prefetchEnd > 1) {
      if (*prefetchEnd < 0) {
        data->AddEmptyMemoryBlock(0, m_left);
      } else {
        data->AddEmptyMemoryBlock(*prefetchEnd + 1, m_left - *prefetchEnd - 1);
      }
    }
    // update the prefetchEnd
    *prefetchEnd = m_left + actualBlockNum - 1;
  } else {
    // if the current status is not the init mode or these data points cannot
    // be stored as prefetching, store them evenly in the data blocks
    int nowBlockNum = flagNumber & 0x00FFFFFF;
    // allocate empty memory block for this node
    if (nowBlockNum == 0) {
      m_left = data->AllocateMemory(neededBlockNum);
    } else {
      // if this node has been initialized before, we should release its
      // memory before allocating the new empty memory blocks
      if (nowBlockNum != neededBlockNum) {
        if (m_left != -1) {
          data->ReleaseMemory(m_left, nowBlockNum);
        }
        m_left = data->AllocateMemory(neededBlockNum);
      }
    }

    LeafSlots<KeyType, ValueType> tmp;
    int avg = std::max(1.0, ceil(static_cast<double>(size) / neededBlockNum));
    avg = std::min(avg, kMaxBlockCapacity);
    CFArrayType tmpArr;
    data->dataArray[m_left] = LeafSlots<KeyType, ValueType>();

    // store data points evenly in the data array
    for (int i = m_left, j = left, k = 1; j < end; j++, k++) {
      InsertDataBlock(dataset[j], i - m_left, &tmp);
      if (k == avg || j == end - 1) {
        k = 0;
        data->dataArray[i++] = tmp;
        tmp = LeafSlots<KeyType, ValueType>();
        actualBlockNum++;
      }
    }
  }
  for (int i = 0; i < kMaxBlockNum - 1; i++) {
    slotkeys[i] = static_cast<KeyType>(DBL_MAX);
  }

  // update the flagNumber with the flag and the number of data blocks
  flagNumber = (ARRAY_LEAF_NODE << 24) + actualBlockNum;

  if (actualBlockNum <= 1) {
    slotkeys[0] = dataset[left + size - 1].first + 1;
  } else {
    end = m_left + actualBlockNum;
    int j = 0;
    double lastKey = dataset[left].first;
    // store the minimum key value of each data block into slotkeys
    if (perSize[0] > 0) {
      lastKey = data->dataArray[m_left].slots[perSize[0] - 1].first;
    }

    for (int i = m_left + 1; i < end; i++, j++) {
      int tmpSize = perSize[i - m_left];
      if (tmpSize > 0) {
        slotkeys[j] = data->dataArray[i].slots[0].first;
        lastKey = data->dataArray[i].slots[tmpSize - 1].first;
      } else {
        slotkeys[j] = lastKey + 1;
      }
    }
  }

  if (size != actualSize) {
    for (int i = m_left, cnt = 0; i < m_left + actualBlockNum; i++) {
      int tmpSize = perSize[i - m_left];
      for (int j = 0; j < tmpSize; j++) {
        if (dataset[cnt].second == static_cast<ValueType>(DBL_MAX)) {
          for (int num = j; num < perSize[i - m_left] - 1; num++) {
            data->dataArray[i].slots[num] = data->dataArray[i].slots[num + 1];
          }
          data->dataArray[i].slots[static_cast<int>(perSize[i - m_left]) - 1] =
              {static_cast<KeyType>(DBL_MAX), static_cast<ValueType>(DBL_MAX)};
          perSize[i - m_left]--;
        }
        cnt++;
      }
    }
  }
  return true;
}

#endif  // NODES_LEAFNODE_CFARRAY_TYPE_H_
