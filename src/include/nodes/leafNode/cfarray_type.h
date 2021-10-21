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
#ifndef SRC_INCLUDE_NODES_LEAFNODE_CFARRAY_TYPE_H_
#define SRC_INCLUDE_NODES_LEAFNODE_CFARRAY_TYPE_H_

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
   * @param size[in] the size of the data points
   * @return int
   */
  static int CalNeededBlockNum(int size);

  /**
   * @brief extract data points (delete useless gaps and deleted data points)
   *
   * @param data[in] the data array
   * @param blockleft[in] the left index of data blocks managed by the leaf node
   * @param blockright[in] the right index of data blocks managed by the leaf
   * node
   * @param actualSize[out] the actual size of these data points
   * @return DataVectorType : pure data points
   */
  static DataVectorType ExtractDataset(
      const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
      int blockright, int *actualSize);

 public:
  // *** Basic Functions of CF Array Leaf Objects

  /**
   * @brief initialize cf array node
   *
   * @param dataset[in] the dataset
   * @param prefetchIndex[in] the prefetch predicted index of each key value
   * @param start_idx[in] the start index of data points
   * @param size[in] the size of data points
   * @param data[out] the data array
   */
  void Init(const DataVectorType &dataset,
            const std::vector<int> &prefetchIndex, int start_idx, int size,
            DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief Find the corresponding data point of the given key value in the data
   * array and return the index of the data block and the index of the data
   * point in this data block. If unsuccessful, return -1.
   *
   * @param data[in] the data array
   * @param key[in] the given key value
   * @param currblock[out] the index of the data block
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
   * @param datapoint[in] the inserted data point
   * @param data[out] the data array
   * @retval true the insert is successful
   * @retval false the insert fails
   */
  bool Insert(const DataType &datapoint,
              DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief Update the given data point.
   *
   * @param datapoint[in] the updated data point
   * @param data[out] the data array
   * @retval true the update is successful
   * @retval false the update fails
   */
  bool Update(const DataType &datapoint,
              DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief Delete the record of the given key value
   *
   * @param key[in] the given key value
   * @param data[out] the data array
   * @return int: the index of the data point in this data block
   * @retval kMaxBlockCapacity: find the data point unsuccessfully
   */
  bool Delete(const KeyType &key, DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief store data points into the data array
   *
   * @param dataset[in] the dataset
   * @param prefetchIndex[in] the prefetch predicted index of each key value
   * @param isInitMode[in] the current construction mode
   * @param needLeafNum the needed number of leaf nodes
   * @param start_idx[in] the start index of data points
   * @param size[in]  the size of data points
   * @param data[out] the data array
   * @param prefetchEnd[out] the right index of prefetched blocks in initial
   * mode
   */
  bool StoreData(const DataVectorType &dataset,
                 const std::vector<int> &prefetchIndex, bool isInitMode,
                 int needLeafNum, int start_idx, int size,
                 DataArrayStructure<KeyType, ValueType> *data,
                 int *prefetchEnd);

 public:
  //*** Auxiliary Functions

  /**
   * @brief Get the number of data points in data.dataArray[blockleft,
   * blockright)
   *
   * @param data[in] the data array
   * @param blockleft[in] the left index of the data blocks
   * @param blockright[in] the right index of the data blocks
   * @return int: the number of data points
   */
  inline int GetDataNum(const DataArrayStructure<KeyType, ValueType> &data,
                        int blockleft, int blockright);

 public:
  //*** Sub-Functions of Find

  /**
   * @brief Compare the given key value with slotkeys to search the index of the
   * corresponding data block of the given key value. This function is the
   * sub-function of find function, and we set it as public due to the need
   * to test the CPU time.
   *
   * @param key[in] the given key value
   * @return int: the index of the data block
   */
  int Search(const KeyType &key) const;

  /**
   * @brief Search the data point of the given key value in the given data block
   * and return the index of it. If unsuccessfuly, return kMaxBlockCapacity.This
   * function is the sub-function of find function, and we set it as public due
   * to the need to test the CPU time.
   *
   * @param block[in] the current data block
   * @param key[in] the given key value
   * @return int: the index of the data point in this data block
   * @retval kMaxBlockCapacity: if we fail to find the corresponding data
   * point, return kMaxBlockCapacity
   */
  int SearchDataBlock(const LeafSlots<KeyType, ValueType> &block,
                      const KeyType &key) const;

 private:
  //*** Private Sub-Functions of StoreData

  /**
   * @brief check whether this array node can be prefetched
   *
   * @param data[in] the data array
   * @param dataset[in] the dataset
   * @param prefetchIndex[in] the prefetch predicted index of each key value
   * @param neededBlockNum[in] the needed number of leaf nodes
   * @param start_idx[in] the start index of data points
   * @param size[in]  the size of data points
   */
  bool CheckIsPrefetch(const DataArrayStructure<KeyType, ValueType> &data,
                       const DataVectorType &dataset,
                       const std::vector<int> prefetchIndex, int neededBlockNum,
                       int start_idx, int size);

  /**
   * @brief store data points in such a way that extra data points are filled
   * with the previous blocks first
   *
   * @param dataset[in] the dataset to be stored
   * @param prefetchIndex[in] the prefetch predicted index of each key value
   * @param neededBlockNum[in] the number of needed data blocks to store all the
   * data points
   * @param tmpBlockVec[out] blocks to store data points
   * @param actualBlockNum[out] the actual number of used data blocks
   * @param missNumber[out] the number of data points which are not stored as
   * the prefetching index
   * @retval true succeeds
   * @return false fails
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
   * @param dataset[in] the dataset to be stored
   * @param prefetchIndex[in] the prefetch predicted index of each key value
   * @param neededBlockNum[in] the number of needed data blocks to store all the
   * data points
   * @param tmpBlockVec[out] blocks to store data points
   * @param actualBlockNum[out] the actual number of used data blocks
   * @param missNumber[out] the number of data points which are not stored as
   * the prefetching index
   * @retval true succeeds
   * @return false fails
   */
  inline bool StoreSubsequent(
      const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
      int neededBlockNum,
      std::vector<LeafSlots<KeyType, ValueType>> *tmpBlockVec,
      int *actualBlockNum, int *missNumber);

 private:
  // *** Private functions for insert operations

  /**
   * @brief If the data block to be inserted is full and the next data block
   * is also saturated, the rebalance mechanism is triggered. The leaf node
   * collects all data points in all managed data blocks and reallocates them
   * evenly to all these blocks.
   *
   * @param blockleft[in] the left index of data blocks
   * @param blockright[in] the right index of data blocks
   * @param data[out] the data array
   */
  inline void Rebalance(int blockleft, int blockright,
                        DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief When this leaf node needs more space to store more data points,
   * the expand operation can be initiated to get more data blocks. We first
   * collect all the data points managed by this leaf node, and then get a new
   * location with more space in the data array.
   *
   * @param blockleft[in] the left index of data blocks
   * @param blockright[in] the right index of data blocks
   * @param data[out] the data array
   */
  inline void Expand(int blockleft, int blockright,
                     DataArrayStructure<KeyType, ValueType> *data);

  /**
   * @brief insert the given data into a leaf node
   *
   * @param datapoint[in] the inserted data poinys
   * @param currblock[in] the index of current structure
   * @param currBlock[out] the data block where the data will be inserted
   * @param cfNode[out] the cf array node
   * @retval true succeeds
   * @retval false fails
   */

  bool InsertDataBlock(const DataType &datapoint, int currblock,
                       LeafSlots<KeyType, ValueType> *currBlock,
                       CFArrayType *cfNode);

  /**
   * @brief insert a data point into the next structure in the leaf node
   *
   * @param datapoint[in] the data points needed to be inserted
   * @param nowDataIdx[in] the index of this structure in data
   * @param currblock[in] the index of the current structure in the leaf node
   * @param data[in] the data array
   * @param cfNode[out] the leaf node
   * @retval true INSERT succeeds
   * @retval false INSERT fails (the next structure is full)
   */
  inline bool InsertNextBlock(const DataType &currdata, int nowDataIdx,
                              int currblock,
                              DataArrayStructure<KeyType, ValueType> *data,
                              CFArrayType *cfNode);

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
#endif  // DEBUG
  int neededBlockNumber = std::min(
      static_cast<int>(ceil(size * 1.0 / kMaxBlockCapacity)), kMaxBlockNum);

  return neededBlockNumber;
}

template <typename KeyType, typename ValueType>
typename CFArrayType<KeyType, ValueType>::DataVectorType
CFArrayType<KeyType, ValueType>::ExtractDataset(
    const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
    int blockright, int *actual) {
  *actual = 0;
  int size = blockright - blockleft;
  int maxBlockNum = carmi_params::kMaxLeafNodeSize / sizeof(DataType);
  DataVectorType currdata(size * maxBlockNum, {DBL_MAX, DBL_MAX});
  for (int i = blockleft; i < blockright; i++) {
    for (int j = 0; j < maxBlockNum; j++) {
      if (data.dataArray[i].slots[j].first != DBL_MAX) {
        currdata[(*actual)++] = data.dataArray[i].slots[j];
      }
    }
  }
  return currdata;
}

template <typename KeyType, typename ValueType>
inline void CFArrayType<KeyType, ValueType>::Init(
    const DataVectorType &dataset, const std::vector<int> &prefetchIndex,
    int start_idx, int size, DataArrayStructure<KeyType, ValueType> *data) {
  int actualSize = 0;

  DataVectorType currdata(size, {DBL_MAX, DBL_MAX});
  int end = start_idx + size;
  for (int i = start_idx; i < end; i++) {
    if (dataset[i].first != DBL_MAX) {
      currdata[actualSize++] = dataset[i];
    }
  }
  int neededBlockNum = CalNeededBlockNum(size);

  StoreData(currdata, prefetchIndex, neededBlockNum, 0.9, 0, actualSize, data,
            0);
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
  bool isSuccess = InsertDataBlock(
      datapoint, currblock, &(data->dataArray[m_left + currblock]), this);
  if (isSuccess) {
    return true;
  }

  int nowDataNum = GetDataNum(*data, m_left, m_left + nowBlockNum);

  // currBlockIdx is the index of this data block in the data array
  int currBlockIdx = m_left + currblock;

  // Case 4: insert into the next data block.
  // If inserting into the current data block fails, and there is an empty
  // position in the next data block, insert into the next data block.
  // Return directly if successful.
  if (currblock < nowBlockNum - 1 &&
      data->dataArray[currBlockIdx + 1].slots[kMaxBlockCapacity - 1].first ==
          DBL_MAX) {
    isSuccess = InsertNextBlock(datapoint, currBlockIdx, currblock, data, this);
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
                              &(data->dataArray[m_left + currblock]), this);

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
inline void CFArrayType<KeyType, ValueType>::Rebalance(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType> *data) {
  int actualSize = 0;
  DataVectorType newDataset =
      ExtractDataset(*data, blockleft, blockright, &actualSize);
  int nowBlockNum = blockright - blockleft;
  std::vector<int> prefetchIndex(actualSize, 0);

  StoreData(newDataset, prefetchIndex, false, nowBlockNum, 0, actualSize, data,
            0);
}

template <typename KeyType, typename ValueType>
inline void CFArrayType<KeyType, ValueType>::Expand(
    int blockleft, int blockright,
    DataArrayStructure<KeyType, ValueType> *data) {
  int actualSize = 0;
  DataVectorType newDataset =
      ExtractDataset(*data, blockleft, blockright, &actualSize);
  int neededBlockNum = blockright - blockleft + 1;
  std::vector<int> prefetchIndex(actualSize, 0);

  StoreData(newDataset, prefetchIndex, false, neededBlockNum, 0, actualSize,
            data, 0);
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::CheckIsPrefetch(
    const DataArrayStructure<KeyType, ValueType> &data,
    const DataVectorType &dataset, const std::vector<int> prefetchIndex,
    int neededBlockNum, int left, int size) {
  int end = left + size;
  int leftIdx = prefetchIndex[0];
  // check whether these points can be prefetched
  if (leftIdx + neededBlockNum > data.dataArray.size()) {
    return false;
  }
  int rightIdx = prefetchIndex[size - 1];
  rightIdx = std::min(leftIdx + neededBlockNum - 1, rightIdx);
  for (int i = leftIdx; i <= rightIdx; i++) {
    if (data.dataArray[i].slots[0].first != DBL_MAX) {
      return false;
    }
  }
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
  CFArrayType tmpArr;
  for (int i = 0; i < size; i++) {
    int p = prefetchIndex[i];
    p -= leftIdx;
    if (p >= neededBlockNum) {
      p = neededBlockNum - 1;
    }
    auto insertSuccess =
        InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p], &tmpArr);
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
      insertSuccess =
          InsertDataBlock(preDatapoint, 0, &(*tmpBlockVec)[p], &tmpArr);
      nowDatapoint = preDatapoint;
    }
    // insert into the subsequent block
    if (!insertSuccess) {
      p = tmpP;
      while (p + 1 < neededBlockNum && !insertSuccess) {
        *tmpBlockVec = tmp;
        p++;
        insertSuccess =
            InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p], &tmpArr);
      }
    }
    if (insertSuccess) {
      if (p + 1 > *actualBlockNum) {
        *actualBlockNum = p + 1;
      }
    } else {
      return false;
    }
  }

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
  CFArrayType tmpArr;
  for (int i = 0; i < size; i++) {
    int p = prefetchIndex[i];
    p -= leftIdx;
    if (p >= neededBlockNum) {
      p = neededBlockNum - 1;
    }
    auto insertSuccess =
        InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p], &tmpArr);
    DataType nowDatapoint = dataset[i];
    DataType preDatapoint;
    // insert into the subsequent block
    while (!insertSuccess && p + 1 < neededBlockNum) {
      p++;
      insertSuccess =
          InsertDataBlock(dataset[i], 0, &(*tmpBlockVec)[p], &tmpArr);
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
      insertSuccess =
          InsertDataBlock(preDatapoint, 0, &(*tmpBlockVec)[p], &tmpArr);
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
  if (neededBlockNum == 0 || size == 0) {
    flagNumber = (ARRAY_LEAF_NODE << 24) + 0;
    return true;
  }
  int end = left + size;
  int actualNum = 0;
  bool isPossible = true;
  int maxBlockNum = carmi_params::kMaxLeafNodeSize / sizeof(DataType);
  int leftIdx = prefetchIndex[0];
  // check whether these points can be prefetched
  if (isInitMode) {
    isPossible = CheckIsPrefetch(*data, dataset, prefetchIndex, neededBlockNum,
                                 left, size);
  }
  if (isInitMode && isPossible) {
    m_left = leftIdx;

    DataVectorType tmpDataset(dataset.begin() + left, dataset.begin() + end);

    LeafSlots<KeyType, ValueType> tmpSlot;
    std::vector<LeafSlots<KeyType, ValueType>> prevBlocks(neededBlockNum,
                                                          tmpSlot);
    int prevActualNum = 0;
    int prevMissNum = 0;
    bool isPreviousSuccess =
        StorePrevious(tmpDataset, prefetchIndex, neededBlockNum, &prevBlocks,
                      &prevActualNum, &prevMissNum);

    std::vector<LeafSlots<KeyType, ValueType>> nextBlocks(neededBlockNum,
                                                          tmpSlot);
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
      return false;
    }
    int tmpMissNum = 0;
    if (isPrev) {
      actualNum = prevActualNum;
      tmpMissNum = prevMissNum;
      for (int i = m_left; i < m_left + actualNum; i++) {
        data->dataArray[i] = prevBlocks[i - m_left];
      }
    } else {
      actualNum = nextActualNum;
      tmpMissNum = nextMissNum;
      for (int i = m_left; i < m_left + actualNum; i++) {
        data->dataArray[i] = nextBlocks[i - m_left];
      }
    }

    if (m_left + actualNum > static_cast<int>(data->usedDatasize)) {
      data->usedDatasize = m_left + actualNum;
    }
    if (m_left - *prefetchEnd > 1) {
      if (*prefetchEnd < 0) {
        data->AddEmptyMemoryBlock(0, m_left);
      } else {
        data->AddEmptyMemoryBlock(*prefetchEnd + 1, m_left - *prefetchEnd - 1);
      }
    }
    *prefetchEnd = m_left + actualNum - 1;

  } else {
    // for EXPAND
    int nowBlockNum = flagNumber & 0x00FFFFFF;
    if (nowBlockNum == 0) {
      m_left = data->AllocateMemory(neededBlockNum);
    } else {
      if (nowBlockNum != neededBlockNum) {
        if (m_left != -1) {
          data->ReleaseMemory(m_left, nowBlockNum);
        }
        m_left = data->AllocateMemory(neededBlockNum);
      }
    }

    LeafSlots<KeyType, ValueType> tmp;
    int avg = std::max(1.0, ceil(size * 1.0 / neededBlockNum));
    avg = std::min(avg, maxBlockNum);
    CFArrayType tmpArr;
    data->dataArray[m_left] = LeafSlots<KeyType, ValueType>();

    for (int i = m_left, j = left, k = 1; j < end; j++, k++) {
      InsertDataBlock(dataset[j], 0, &tmp, &tmpArr);
      if (k == avg || j == end - 1) {
        k = 0;
        data->dataArray[i++] = tmp;
        tmp = LeafSlots<KeyType, ValueType>();
        actualNum++;
      }
    }
  }

  flagNumber = (ARRAY_LEAF_NODE << 24) + actualNum;

  if (actualNum <= 1) {
    slotkeys[0] = dataset[left + size - 1].first + 1;
    return true;
  }
  end = m_left + actualNum;
  int j = 0;
  double lastKey = dataset[left].first;
  for (int i = 0; i < maxBlockNum; i++) {
    if (data->dataArray[m_left].slots[i].first != DBL_MAX) {
      lastKey = data->dataArray[m_left].slots[i].first;
    } else {
      break;
    }
  }
  for (int i = m_left + 1; i < end; i++, j++) {
    if (data->dataArray[i].slots[0].first != DBL_MAX) {
      slotkeys[j] = data->dataArray[i].slots[0].first;
      for (int k = maxBlockNum - 1; k >= 0; k--) {
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

template <typename KeyType, typename ValueType>
inline int CFArrayType<KeyType, ValueType>::GetDataNum(
    const DataArrayStructure<KeyType, ValueType> &data, int blockleft,
    int blockright) {
  int num = 0;
  int maxBlockNum = carmi_params::kMaxLeafNodeSize / sizeof(DataType);
  for (int i = blockleft; i < blockright; i++) {
    for (int j = 0; j < maxBlockNum; j++) {
      if (data.dataArray[i].slots[j].first != DBL_MAX) {
        num++;
      }
    }
  }
  return num;
}

template <typename KeyType, typename ValueType>
bool CFArrayType<KeyType, ValueType>::InsertDataBlock(
    const DataType &currdata, int currblock,
    LeafSlots<KeyType, ValueType> *currBlock, CFArrayType *cfNode) {
  if (currBlock->slots[kMaxBlockCapacity - 1].first != DBL_MAX) {
    return false;  // this node is full
  }
  if (currBlock->slots[0].first == DBL_MAX) {
    currBlock->slots[0] = currdata;
    if (currblock == 0) {
      cfNode->slotkeys[0] = currdata.first + 1;
    } else {
      cfNode->slotkeys[currblock - 1] = currdata.first;
    }
    return true;
  }
  int res = SearchDataBlock(*currBlock, currdata.first);
  if (currBlock->slots[res].first == DBL_MAX) {
    currBlock->slots[res] = currdata;
    if (currblock != 0) {
      cfNode->slotkeys[currblock - 1] =
          std::min(cfNode->slotkeys[currblock - 1], currdata.first);
    } else {
      if (currdata.first > cfNode->slotkeys[0]) {
        cfNode->slotkeys[0] = currdata.first + 1;
      }
    }
    return true;
  }
  // move data
  int num = res;
  for (; num < kMaxBlockCapacity; num++) {
    if (currBlock->slots[num].first == DBL_MAX) {
      break;
    }
  }
  for (; num > res; num--) {
    currBlock->slots[num] = currBlock->slots[num - 1];
  }
  currBlock->slots[res] = currdata;
  if (currblock != 0) {
    cfNode->slotkeys[currblock - 1] =
        std::min(cfNode->slotkeys[currblock - 1], currdata.first);
  } else {
    if (currdata.first > cfNode->slotkeys[0]) {
      cfNode->slotkeys[0] = currdata.first + 1;
    }
  }
  return true;
}

template <typename KeyType, typename ValueType>
inline bool CFArrayType<KeyType, ValueType>::InsertNextBlock(
    const DataType &currdata, int nowDataIdx, int currblock,
    DataArrayStructure<KeyType, ValueType> *data, CFArrayType *cfNode) {
  int maxBlockNum = carmi_params::kMaxLeafNodeSize / sizeof(DataType);
  if (currdata.first >
      data->dataArray[nowDataIdx].slots[maxBlockNum - 1].first) {
    InsertDataBlock(currdata, currblock + 1, &data->dataArray[nowDataIdx + 1],
                    cfNode);
    cfNode->slotkeys[currblock] =
        data->dataArray[nowDataIdx + 1].slots[0].first;

    return true;
  } else if (currdata.first <
             data->dataArray[nowDataIdx].slots[maxBlockNum - 1].first) {
    InsertDataBlock(data->dataArray[nowDataIdx].slots[maxBlockNum - 1],
                    currblock + 1, &data->dataArray[nowDataIdx + 1], cfNode);

    if (data->dataArray[nowDataIdx].slots[0].first > currdata.first) {
      for (int j = maxBlockNum - 1; j > 0; j--) {
        data->dataArray[nowDataIdx].slots[j] =
            data->dataArray[nowDataIdx].slots[j - 1];
      }
      data->dataArray[nowDataIdx].slots[0] = currdata;
      if (currblock != 0)
        cfNode->slotkeys[currblock - 1] =
            data->dataArray[nowDataIdx].slots[0].first;
    } else {
      for (int i = maxBlockNum - 2; i >= 0; i--) {
        if (data->dataArray[nowDataIdx].slots[i].first <= currdata.first) {
          for (int j = maxBlockNum - 1; j > i + 1; j--) {
            data->dataArray[nowDataIdx].slots[j] =
                data->dataArray[nowDataIdx].slots[j - 1];
          }
          data->dataArray[nowDataIdx].slots[i + 1] = currdata;
          break;
        }
      }
    }
    cfNode->slotkeys[currblock] =
        data->dataArray[nowDataIdx + 1].slots[0].first;
    return true;
  }
  return false;
}

#endif  // SRC_INCLUDE_NODES_LEAFNODE_CFARRAY_TYPE_H_
