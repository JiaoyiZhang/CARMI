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
#include <map>
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
 */
template <typename KeyType, typename ValueType>
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
   * data points actually. The data points are compactly stored in data blocks
   * in a sequential manner in the second layer.
   *
   * When searching for data points in this node, we first search sequentially
   * in the slotkeys, and then search in a data block.
   */
  CFArrayType() {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
    m_left = -1;
    previousLeaf = -1;
    nextLeaf = -1;
    for (int i = 0; i < 48 / static_cast<int>(sizeof(KeyType)); i++) {
      slotkeys[i] = DBL_MAX;
    }
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
   * @brief Get the actual number of data points in data.dataArray[blockleft,
   * blockright)
   *
   * @param[in] data the data array
   * @param[in] blockleft the left index of the data blocks
   * @param[in] blockright the right index of the data blocks
   * @return int: the number of data points
   */
  static int GetDataNum(const DataArrayStructure<KeyType, ValueType> &data,
                        int blockleft, int blockright);

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
      const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
      int blockright);

 public:
  // *** Basic Functions of CF Array Leaf Objects

  /**
   * @brief initialize cf array node and store the given dataset into data
   * blocks
   *
   * @param[in] dataset the dataset
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] start_idx the starting index of data points
   * @param[in] size the size of data points
   * @param[in] data the data array
   */
  void Init(const DataVectorType &dataset,
            const std::vector<int> &prefetchIndex, int start_idx, int size,
            DataArrayStructure<KeyType, ValueType> *data);

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
  int Find(const DataArrayStructure<KeyType, ValueType> &data,
           const KeyType &key, int *currblock) const;

  /**
   * @brief Insert the datapoint into this leaf node
   *
   * The basic process of the insert operation is similar to the lookup
   * operation. After finding the correct data block for insertion, we insert
   * the data point into it. In addition, there are two mechanisms that can be
   * initiated by the leaf node under certain situations:
   *
   * @param[in] datapoint the inserted data point
   * @param[inout] data the data array after the insert
   * @retval true the insert is successful
   * @retval false the insert fails
   */
  bool Insert(const DataType &datapoint,
              DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief Update the given data point.
   *
   * The basic process of the update operation is similar to the find operation.
   * After finding the correct data block, we find the data point regarding to
   * the given key value and update it. If we fails to find the corresponding
   * data point, return false.
   *
   * @param[in] datapoint the updated data point
   * @param[inout] data the data array after the update
   * @retval true the update is successful
   * @retval false the update fails, unable to find the record of the given key
   * value
   */
  bool Update(const DataType &datapoint,
              DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief Delete the record of the given key value
   *
   * The basic process of the delete operation is similar to the update
   * operation. After finding the correct data block, we find all the data
   * points regarding to the given key value and delete them. If there is no
   * corresponding data point, return true directly.
   *
   * @param[in] key the given key value
   * @param[inout] data the data array after the delete
   * @retval true the delete is successful
   */
  bool Delete(const KeyType &key, DataArrayStructure<KeyType, ValueType> *data);

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
   * @param[in] size the size of data points needed to be stored in data array
   * @param[inout] data the data array
   * @param[inout] prefetchEnd the right index of prefetched blocks in initial
   * mode, this parameter is useless if the isInitMode is false.
   * @return true store data points in the data array successfully
   * @return false store data points unsuccessfully, this status only occurs in
   * the init mode and the given data blocks cannot accomodate these data
   * points.
   */
  bool StoreData(const DataVectorType &dataset,
                 const std::vector<int> &prefetchIndex, bool isInitMode,
                 int neededBlockNum, int left, int size,
                 DataArrayStructure<KeyType, ValueType> *data,
                 int *prefetchEnd);

 public:
  //*** Sub-Functions of Find

  /**
   * @brief Compare the given key value with slotkeys to search the index of the
   * corresponding data block of the given key value. This function is the
   * sub-function of find function, and we set it as public due to the need
   * to test the CPU time.
   *
   * @param[in] key the given key value
   * @return int: the index of the data block
   */
  int Search(const KeyType &key) const;

  /**
   * @brief Search the data point of the given key value in the given data block
   * and return the index of it. If unsuccessfuly, return kMaxBlockCapacity.This
   * function is the sub-function of find function, and we set it as public due
   * to the need to test the CPU time.
   *
   * @param[in] block the current data block
   * @param[in] key the given key value
   * @return int: the index of the data point in this data block
   * @retval kMaxBlockCapacity: if we fail to find the corresponding data
   * point, return kMaxBlockCapacity
   */
  int SearchDataBlock(const LeafSlots<KeyType, ValueType> &block,
                      const KeyType &key) const;

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
                        DataArrayStructure<KeyType, ValueType> *data);

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
                     DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief insert the given data point into the data block
   *
   * @param[in] datapoint the inserted data point
   * @param[in] currBlockIdx the index of the current data blocks in this node
   * @param[inout] currBlock the data block into which the data point will be
   * inserted
   * @retval true this operation is successful
   * @retval false if this data block is full, return false
   */
  bool InsertDataBlock(const DataType &datapoint, int currBlockIdx,
                       LeafSlots<KeyType, ValueType> *currBlock);

  /**
   * @brief insert a data point into the next data block in the leaf node
   *
   * @param[in] datapoint the data points needed to be inserted
   * @param[in] currBlockIdx the index of the current data block in the leaf
   * node
   * @param[inout] data the data array
   * @retval true INSERT succeeds
   * @retval false INSERT fails (the next structure is full)
   */
  inline bool InsertNextBlock(const DataType &currdata, int currBlockIdx,
                              DataArrayStructure<KeyType, ValueType> *data);

 private:
  //*** Private Sub-Functions of StoreData

  /**
   * @brief check whether these data points can be stored as prefetched
   *
   * @param[in] data the data array
   * @param[in] dataset the dataset
   * @param[in] prefetchIndex the prefetch predicted index of each key value
   * @param[in] neededBlockNum the needed number of data blocks
   * @param[in] start_idx the starting index of data points in the dataset
   * @param[in] size  the size of data points
   * @retval true these data points can be stored as prefetched
   * @return false these data points cannot be stored as prefetched
   */
  bool CheckIsPrefetch(const DataArrayStructure<KeyType, ValueType> &data,
                       const DataVectorType &dataset,
                       const std::vector<int> &prefetchIndex,
                       int neededBlockNum, int start_idx, int size);

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
   * @return false the given data blocks cannot accomodate these data points
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
   * @param[in] tmpBlockVec blocks to store data points
   * @param[in] actualBlockNum the actual number of used data blocks
   * @param[in] missNumber the number of data points which are not stored as
   * the prefetching index
   * @retval true these data points have been stored as prefetched
   * @return false the given data blocks cannot accomodate these data points
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
  static constexpr int kMaxBlockNum = 48 / sizeof(KeyType) + 1;

  /**
   * @brief CF array leaf node parameter: The maximum number of data points in
   * each cf array leaf node, which is the result of kMaxBlockCapacity *
   * kMaxBlockNum.
   */
  static constexpr int kMaxLeafCapacity = kMaxBlockCapacity * kMaxBlockNum;

 public:
  //*** Public Data Members of CF Array Leaf Node Objects

  /**
   * @brief  A combined integer, composed of the flag of CF array leaf node and
   * the number of data blocks managed by it. (4 bytes)
   */
  int flagNumber;

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
   * @brief The first index of data blocks managed by this object in the data
   * array. (4 bytes)
   */
  int m_left;

  /**
   * @brief An array of length kMaxBlockNum-1, used to store the minimum key
   * value in each data block managed by this object except the first data
   * block. When searching for a data point with a given key value, first find
   * the corresponding data block by sequentially searching in this array.  (48
   * bytes)
   */
  KeyType slotkeys[48 / sizeof(KeyType)];
};

template <typename KeyType, typename ValueType>
inline int CFArrayType<KeyType, ValueType>::CalNeededBlockNum(int size) {
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

template <typename KeyType, typename ValueType>
int CFArrayType<KeyType, ValueType>::GetDataNum(
    const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
    int blockright) {
  int num = 0;
  for (int i = blockleft; i < blockright; i++) {
    for (int j = 0; j < kMaxBlockCapacity; j++) {
      if (data.dataArray[i].slots[j].first != DBL_MAX) {
        num++;
      }
    }
  }
  return num;
}

template <typename KeyType, typename ValueType>
typename CFArrayType<KeyType, ValueType>::DataVectorType
CFArrayType<KeyType, ValueType>::ExtractDataset(
    const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
    int blockright) {
  DataVectorType currdata;
  for (int i = blockleft; i < blockright; i++) {
    for (int j = 0; j < kMaxBlockCapacity; j++) {
      if (data.dataArray[i].slots[j].first != DBL_MAX) {
        currdata.push_back(data.dataArray[i].slots[j]);
      }
    }
  }
  return currdata;
}

template <typename KeyType, typename ValueType>
inline void CFArrayType<KeyType, ValueType>::Init(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int start_idx, int size, DataArrayStructure<KeyType, ValueType> *data) {
  int neededBlockNum = CalNeededBlockNum(size);
  StoreData(dataset, prefetchIndex, false, neededBlockNum, start_idx, size,
            data, 0);
}

template <typename KeyType, typename ValueType>
inline int CFArrayType<KeyType, ValueType>::Find(
    const DataArrayStructure<KeyType, ValueType> &data, const KeyType &key,
    int *currblock) const {
  // Use Search function to find the data block which stores the data point of
  // the given key value.
  // currblock is the serial number of this data block in all data blocks
  // managed by this leaf node.
  *currblock = Search(key);

  // Search the data point in this data block
  int res = SearchDataBlock(data.dataArray[m_left + *currblock], key);

  // Return the index of the data point. If res is equal to kMaxBlockCapacity,
  // finding the corresponding data point fails.
  return res;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::Insert(
    const DataType &datapoint, DataArrayStructure<KeyType, ValueType> *data) {
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  // Case 1: insert fails.
  // If there is no empty position for the newly inserted data point,
  // return false directly.
  if (nowBlockNum >= kMaxLeafCapacity) {
    return false;
  }

  // Case 2: this node is empty.
  // If this node is empty, we first need to allocate a block of memory for it
  // and use the first index to update m_left, then insert the data point in the
  // first allocated data block. After that, we update the number of data blocks
  // managed by this node and return directly.
  if (nowBlockNum == 0) {
    m_left = data->AllocateMemory(1);
    data->dataArray[m_left].slots[0] = datapoint;
    slotkeys[0] = datapoint.first + 1;
    flagNumber++;
    return true;
  }

  // Case 3: insert into the current data block.
  // Use Search function to find the corresponding data block to store this data
  // point. currblock is the serial number of this data block in all data blocks
  // managed by this leaf node. Then insert the data point into this data block,
  // return directly if successful.
  int currblock = Search(datapoint.first);
  bool isSuccess = InsertDataBlock(datapoint, currblock,
                                   &(data->dataArray[m_left + currblock]));
  if (isSuccess) {
    return true;
  }

  int nowDataNum = GetDataNum(*data, m_left, m_left + nowBlockNum);

  // Case 4: insert into the next data block.
  // If inserting into the current data block fails, and there is an empty
  // position in the next data block, insert into the next data block.
  // Return directly if successful.
  if (currblock < nowBlockNum - 1 && data->dataArray[m_left + currblock + 1]
                                             .slots[kMaxBlockCapacity - 1]
                                             .first == DBL_MAX) {
    isSuccess = InsertNextBlock(datapoint, currblock, data);
    if (isSuccess) {
      return true;
    }
  }

  // Case 5: If the latter data block has no empty position, we first need some
  // additional mechanisms to make room for the inserted data point.
  if (nowDataNum < nowBlockNum * (kMaxBlockCapacity - 1)) {
    // Case 5.1: rebalance.
    // If the data block to be inserted is full and the next data block is also
    // saturated, the rebalance mechanism is triggered. Through this mechnism,
    // we can reallocate the data points evenly to all the data blocks.
    Rebalance(m_left, m_left + nowBlockNum, data);
  } else if (nowBlockNum == kMaxBlockNum) {
    // Case 5.2: return false.
    // If the data blocks have reached the upper limit, this node cannot expand.
    // Return false directly.
    return false;
  } else {
    // Case 5.3: expand.
    // This leaf node needs more space to store data points, we can initiate
    // the expand operation to get more data blocks.
    Expand(m_left, m_left + nowBlockNum, data);
  }
  // After the corresponding mechnism, there are some empty positions for the
  // inserted data point.
  currblock = Search(datapoint.first);
  isSuccess = InsertDataBlock(datapoint, currblock,
                              &(data->dataArray[m_left + currblock]));

  return isSuccess;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::Update(
    const DataType &datapoint, DataArrayStructure<KeyType, ValueType> *data) {
  // Get the number of data blocks
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  // Find the corresponding data block which stores this data point.
  int currBlockIdx = m_left + Search(datapoint.first);
  // Search the data point in this data block
  int res = SearchDataBlock(data->dataArray[currBlockIdx], datapoint.first);

  // Update the value and return the status.
  if (data->dataArray[currBlockIdx].slots[res].first == datapoint.first) {
    data->dataArray[currBlockIdx].slots[res].second = datapoint.second;
    return true;
  }
  return false;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::Delete(
    const KeyType &key, DataArrayStructure<KeyType, ValueType> *data) {
  // Get the number of data blocks
  int nowBlockNum = flagNumber & 0x00FFFFFF;
  // Find the corresponding data block which stores this data point.
  int currBlockIdx = m_left + Search(key);
  // Search the data point in this data block
  int res = SearchDataBlock(data->dataArray[currBlockIdx], key);

  // Delete all the data points related to the given key value.
  // deletedCnt is the number of all deleted data points.
  int deletedCnt = 0;
  for (int i = currBlockIdx, j = res; i < nowBlockNum && j < kMaxBlockCapacity;
       j++) {
    if (data->dataArray[i].slots[j].first == DBL_MAX) {
      // Case 1: this slot is empty.
      // Do nothing and return directly.
      break;
    } else if (data->dataArray[i].slots[j].first == key) {
      // Case 2: this slot stores the data point to be deleted.
      // Clear this data point first and updated the delete counter. If this
      // slot is the last one of this data block, continue to check the next
      // data block.
      data->dataArray[i].slots[j] = {DBL_MAX, DBL_MAX};
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
  return true;
}

template <typename KeyType, typename ValueType>
inline int CFArrayType<KeyType, ValueType>::Search(const KeyType &key) const {
  // return the idx of the block in data
  int end_idx = (flagNumber & 0x00FFFFFF) - 1;
  for (int i = 0; i < end_idx; i++) {
    if (key < slotkeys[i]) {
      return i;
    }
  }
  return end_idx;
}

template <typename KeyType, typename ValueType>
inline int CFArrayType<KeyType, ValueType>::SearchDataBlock(
    const LeafSlots<KeyType, ValueType> &currblock, const KeyType &key) const {
  int i = 0;
  if (key <= currblock.slots[kMaxBlockCapacity / 2 - 1].first) {
    for (; i < kMaxBlockCapacity / 2; i += 4) {
      if (key <= currblock.slots[i].first) {
        break;
      }
      if (key <= currblock.slots[i + 1].first) {
        break;
      }
      if (key <= currblock.slots[i + 2].first) {
        break;
      }
      if (key <= currblock.slots[i + 3].first) {
        break;
      }
    }
  } else {
    for (i = kMaxBlockCapacity / 2; i < kMaxBlockCapacity; i += 4) {
      if (key <= currblock.slots[i].first) {
        break;
      }
      if (key <= currblock.slots[i + 1].first) {
        break;
      }
      if (key <= currblock.slots[i + 2].first) {
        break;
      }
      if (key <= currblock.slots[i + 3].first) {
        break;
      }
    }
  }
  return i;
}

template <typename KeyType, typename ValueType>
inline void CFArrayType<KeyType, ValueType>::Rebalance(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType> *data) {
  // extract pure data points
  DataVectorType newDataset = ExtractDataset(*data, blockleft, blockright);
  int nowBlockNum = blockright - blockleft;
  std::vector<int> prefetchIndex(newDataset.size(), 0);

  // store data points
  StoreData(newDataset, prefetchIndex, false, nowBlockNum, 0, newDataset.size(),
            data, 0);
}

template <typename KeyType, typename ValueType>
inline void CFArrayType<KeyType, ValueType>::Expand(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType> *data) {
  // extract pure data points
  DataVectorType newDataset = ExtractDataset(*data, blockleft, blockright);
  int neededBlockNum = blockright - blockleft + 1;
  std::vector<int> prefetchIndex(newDataset.size(), 0);

  // store data points
  StoreData(newDataset, prefetchIndex, false, neededBlockNum, 0,
            newDataset.size(), data, 0);
}

template <typename KeyType, typename ValueType>
bool CFArrayType<KeyType, ValueType>::InsertDataBlock(
    const DataType &currdata, int currBlockIdx,
    LeafSlots<KeyType, ValueType> *currBlock) {
  // Case 1: this data block is full, return false directly
  if (currBlock->slots[kMaxBlockCapacity - 1].first != DBL_MAX) {
    return false;
  }
  // Case 2: this first slot in the currBlock is empty, insert the data point
  // into it and update the slotkeys, then return true directly
  if (currBlock->slots[0].first == DBL_MAX) {
    currBlock->slots[0] = currdata;
    if (currBlockIdx == 0) {
      slotkeys[0] = currdata.first + 1;
    } else {
      slotkeys[currBlockIdx - 1] = currdata.first;
    }
    return true;
  }
  // Case 3: insert the data point into other slots
  // first find the position to be inserted
  int res = SearchDataBlock(*currBlock, currdata.first);
  // Case 3.1: this slot is empty, insert it directly
  if (currBlock->slots[res].first == DBL_MAX) {
    currBlock->slots[res] = currdata;
    if (currBlockIdx != 0) {
      slotkeys[currBlockIdx - 1] =
          std::min(slotkeys[currBlockIdx - 1], currdata.first);
    } else {
      if (currdata.first > slotkeys[0]) {
        slotkeys[0] = currdata.first + 1;
      }
    }
    return true;
  }
  // Case 3.2: this slot is not empty
  // We first need to make room for the inserted data point
  int num = res;
  // calculate the index of the last data points needed to be moved
  for (; num < kMaxBlockCapacity; num++) {
    if (currBlock->slots[num].first == DBL_MAX) {
      break;
    }
  }
  // move data points after the inserted slot
  for (; num > res; num--) {
    currBlock->slots[num] = currBlock->slots[num - 1];
  }
  // store the data point into the slot
  currBlock->slots[res] = currdata;
  if (currBlockIdx != 0) {
    slotkeys[currBlockIdx - 1] =
        std::min(slotkeys[currBlockIdx - 1], currdata.first);
  } else {
    if (currdata.first > slotkeys[0]) {
      slotkeys[0] = currdata.first + 1;
    }
  }
  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::InsertNextBlock(
    const DataType &currdata, int currBlockIdx,
    DataArrayStructure<KeyType, ValueType> *data) {
  int nowDataIdx = m_left + currBlockIdx;
  // Case 1: the key value of the inserted data point is larger than the last
  // data point in the current data block, insert the given data point into
  // the next data block
  if (currdata.first >=
      data->dataArray[nowDataIdx].slots[kMaxBlockCapacity - 1].first) {
    InsertDataBlock(currdata, currBlockIdx + 1,
                    &data->dataArray[nowDataIdx + 1]);
  } else {
    // Case 2: the key value of the inserted data point is smaller than the last
    // data point in the current data block, insert the last data point in the
    // current data block into the next data block
    InsertDataBlock(data->dataArray[nowDataIdx].slots[kMaxBlockCapacity - 1],
                    currBlockIdx + 1, &data->dataArray[nowDataIdx + 1]);

    // move data points and store the given data point in this data block
    for (int i = kMaxBlockCapacity - 1; i >= 0; i--) {
      if (i == 0) {
        data->dataArray[nowDataIdx].slots[0] = currdata;
        if (currBlockIdx != 0)
          slotkeys[currBlockIdx - 1] =
              data->dataArray[nowDataIdx].slots[0].first;
      }
      if (data->dataArray[nowDataIdx].slots[i - 1].first > currdata.first) {
        data->dataArray[nowDataIdx].slots[i] =
            data->dataArray[nowDataIdx].slots[i - 1];
      } else {
        data->dataArray[nowDataIdx].slots[i] = currdata;
        break;
      }
    }
  }
  // update the slotkeys
  slotkeys[currBlockIdx] = data->dataArray[nowDataIdx + 1].slots[0].first;
  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::CheckIsPrefetch(
    const DataArrayStructure<KeyType, ValueType> &data,
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int neededBlockNum, int left, int size) {
  // check whether these points can be prefetched
  int end = left + size;
  int leftIdx = prefetchIndex[0];
  // Case 1: the needed data blocks exceed the exsiting data array
  // return false directly
  if (leftIdx + neededBlockNum > data.dataArray.size()) {
    return false;
  }
  int rightIdx = prefetchIndex[size - 1];
  rightIdx = std::min(leftIdx + neededBlockNum - 1, rightIdx);
  // Case 2: if the predicted data blocks have been used before, return false
  for (int i = leftIdx; i <= rightIdx; i++) {
    if (data.dataArray[i].slots[0].first != DBL_MAX) {
      return false;
    }
  }
  // Case 3: these data points can be prefetched
  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::StorePrevious(
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
    auto insertSuccess = InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p]);
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
      insertSuccess = InsertDataBlock(preDatapoint, 0, &(*tmpBlockVec)[p]);
      nowDatapoint = preDatapoint;
    }
    // insert into the subsequent block
    if (!insertSuccess) {
      p = tmpP;
      while (p + 1 < neededBlockNum && !insertSuccess) {
        *tmpBlockVec = tmp;
        p++;
        insertSuccess = InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p]);
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
      if ((*tmpBlockVec)[i].slots[j].first != DBL_MAX) {
        int fetchIdx = prefetchIndex[cnt++];
        fetchIdx -= leftIdx;
        if (fetchIdx != i) {
          tmpMissNum++;
        }
      }
    }
  }
  *missNumber = tmpMissNum;

  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::StoreSubsequent(
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
    auto insertSuccess = InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p]);
    DataType nowDatapoint = dataset[i];
    DataType preDatapoint;
    // insert into the subsequent block
    while (!insertSuccess && p + 1 < neededBlockNum) {
      p++;
      insertSuccess = InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p]);
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
      insertSuccess = InsertDataBlock(preDatapoint, 0, &(*tmpBlockVec)[p]);
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
      if ((*tmpBlockVec)[i].slots[j].first != DBL_MAX) {
        int fetchIdx = prefetchIndex[cnt++];
        fetchIdx -= leftIdx;
        if (fetchIdx != i) {
          tmpMissNum++;
        }
      }
    }
  }

  *missNumber = tmpMissNum;
  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::StoreData(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    bool isInitMode, int neededBlockNum, int left, int size,
    DataArrayStructure<KeyType, ValueType> *data, int *prefetchEnd) {
  // if the dataset is empty, return true directly
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
    isPossible = CheckIsPrefetch(*data, dataset, prefetchIndex, neededBlockNum,
                                 left, size);
  }
  if (isInitMode && isPossible) {
    // if the current status is init mode and these data points can be stored as
    // prefetched, we then store them as the position in the prefetchIndex
    m_left = leftIdx;

    LeafSlots<KeyType, ValueType> tmpSlot;
    DataVectorType tmpDataset(dataset.begin() + left, dataset.begin() + end);
    std::vector<LeafSlots<KeyType, ValueType>> prevBlocks(neededBlockNum,
                                                          tmpSlot);
    // store in a way that gives priority to the previous data blocks
    int prevActualNum = 0;
    int prevMissNum = 0;
    bool isPreviousSuccess =
        StorePrevious(tmpDataset, prefetchIndex, neededBlockNum, &prevBlocks,
                      &prevActualNum, &prevMissNum);

    std::vector<LeafSlots<KeyType, ValueType>> nextBlocks(neededBlockNum,
                                                          tmpSlot);
    // store in a way that gives priority to the subsequent data blocks
    int nextActualNum = 0;
    int nextMissNum = 0;
    bool isNextSuccess =
        StoreSubsequent(tmpDataset, prefetchIndex, neededBlockNum, &nextBlocks,
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
    // if the current status is not the init mode or these data points cannot be
    // stored as prefetching, store them evenly in the data blocks
    int nowBlockNum = flagNumber & 0x00FFFFFF;
    // allocate empty memory block for this node
    if (nowBlockNum == 0) {
      m_left = data->AllocateMemory(neededBlockNum);
    } else {
      // if this node has been initialized before, we should release its memory
      // before allocating the new empty memory blocks
      if (nowBlockNum != neededBlockNum) {
        if (m_left != -1) {
          data->ReleaseMemory(m_left, nowBlockNum);
        }
        m_left = data->AllocateMemory(neededBlockNum);
      }
    }

    LeafSlots<KeyType, ValueType> tmp;
    int avg = std::max(1.0, ceil(size * 1.0 / neededBlockNum));
    avg = std::min(avg, kMaxBlockCapacity);
    CFArrayType tmpArr;
    data->dataArray[m_left] = LeafSlots<KeyType, ValueType>();

    // store data points evenly in the data array
    for (int i = m_left, j = left, k = 1; j < end; j++, k++) {
      InsertDataBlock(dataset[j], 0, &tmp);
      if (k == avg || j == end - 1) {
        k = 0;
        data->dataArray[i++] = tmp;
        tmp = LeafSlots<KeyType, ValueType>();
        actualBlockNum++;
      }
    }
  }
  for (int i = 0; i < 48 / static_cast<int>(sizeof(KeyType)); i++) {
    slotkeys[i] = DBL_MAX;
  }

  // update the flagNumber with the flag and the number of data blocks
  flagNumber = (ARRAY_LEAF_NODE << 24) + actualBlockNum;

  if (actualBlockNum <= 1) {
    slotkeys[0] = dataset[left + size - 1].first + 1;
    return true;
  }
  end = m_left + actualBlockNum;
  int j = 0;
  double lastKey = dataset[left].first;
  // store the minimum key value of each data block into slotkeys
  for (int i = 0; i < kMaxBlockCapacity; i++) {
    if (data->dataArray[m_left].slots[i].first != DBL_MAX) {
      lastKey = data->dataArray[m_left].slots[i].first;
    } else {
      break;
    }
  }
  for (int i = m_left + 1; i < end; i++, j++) {
    if (data->dataArray[i].slots[0].first != DBL_MAX) {
      slotkeys[j] = data->dataArray[i].slots[0].first;
      for (int k = kMaxBlockCapacity - 1; k >= 0; k--) {
        if (data->dataArray[i].slots[k].first != DBL_MAX) {
          lastKey = data->dataArray[i].slots[k].first;
          break;
        }
      }
    } else {
      slotkeys[j] = lastKey + 1;
    }
  }
  return true;
}

#endif  // NODES_LEAFNODE_CFARRAY_TYPE_H_
