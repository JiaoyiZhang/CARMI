# README
# A simple implementation of CARMI

This is a simple implementation of our paper: **CARMI: A Cache-Aware Learned Index with a Cost-based Construction Algorithm**.

## Reproducing the experiment

If you want to reproduce the experiment in our paper, do the following

```
cd ./src
cmake .
make
./CARMI
```

## Using CARMI

If you want to use CARMI as an index, then you only need to include the header file respectively:

Use in-memory index:
```
#include "./include/carmi_common.h"
```

Use external index:
```
#include "./include/carmi_external.h"
```

## Instructions

**Method of constructing an index:**

1. **Automatic construction(CARMI)**: prepare the initial dataset, training datasets (historical access and insertion queries), and then create a CARMI object, and the hybrid algorithm will automatically build the index.

**Main functions:**

1. **find**: find the corresponding record of the given key, return the iterator

```
iterator find(const KeyType &key);
```

2. **lower_bound**: return an iterator pointing to the first element in the container whose key is not less than key.

```
iterator lower_bound(const KeyType &key);
```

3. **upper_bound**: return an iterator pointing to the first element in the container whose key is larger than key.

```
iterator upper_bound(const KeyType &key);
```

4. **insert**: insert a data point into the index.

```
std::pair<iterator, bool> insert(const DataType &datapoint);
```

5. **erase**: delete the record of the given key and return the number of elements erased.

```
size_t erase(const KeyType &key);
```

6. **swap**: swap two carmi tree objects.

```
void swap(CARMIMap &other);
```

7. **size**: return the number of data points in the carmi tree.

```
size_t size();
```

8. **CalculateSpace**: return the space of the carmi tree in bytes.

```
long long CalculateSpace();
```

Only a few commonly used functions are briefly introduced here. In fact, we provide all interfaces similar to std::map in the C++11 version, and you can use CARMIMap like std::map. CARMIExternalMap is designed to store the data points externally. It also implements the std::map interfaces, but the template parameters are slightly different. You can check the examples we provide to use.

## File structure of CARMI

In this project, we include the CARMI header files, the source code of the experimental part and the baseline. The description of each file in CARMI's header file is as follows:

- **include**
  - base_node.h  *( the union structure of all nodes )*
  - carmi.h  *( the implementation class of CARMI )*
  - carmi_map.h  *( the CARMI map class for common use )*
  - carmi_external_map.h  *( the CARMI map class for the dataset stored in the external position )*
  - **construct**  *( files used to construct the index )*
    - construction.h *( the main function of our algorithm )*
    - construct_root.h *( use the optimal root node to construct child nodes )*
    - dp.h *( the main dynamic programming algorithm )*
    - dp_inner.h *( use DP to construct inner node )*
    - dp_leaf.h *( use DP to construct leaf node )*
    - greedy.h *( the greedy node selection algorithm )*
    - minor_function.h *( minor functions )*
    - structures.h *( the structures of CARMI )*
    - store_node.h *( use the optimal setting to construct a new node )*
  - **memoryLayout**  *( manage the two main arrays )*
    - data_array.h
    - node_array.h
    - empty_block.h
  - **nodes**   *( all nodes we have implemented )*
    - **rootNode**
      - root_nodes.h  *( the classes of root nodes )*
      - **trainModel** *( models used to train the root nodes )*
        - linear_regression.h
        - piecewiseLR.h
        - prefetch_plr.h
    - **innerNode**
      - lr_model.h
      - plr_model.h
      - his_model.h
      - bs_model.h
      - candidate_plr.h *( for piecewiseLR )*
    - **leafNode**
      - cfarray_type.h
      - external_array_type.h
  - **func**  *( public functions )*
    - find_function.h
    - insert_function.h
    - delete_function.h
    - update_function.h
    - split_function.h
    - calculate_space.h
    - get_node_info.h

## Dependencies

This code is based on C++17.
