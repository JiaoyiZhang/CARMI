# A simple implementation of CARMI

This is a simple implementation of our paper: **CARMI: A Cache-Aware Learned Index with a Cost-based Construction Algorithm**.

## Reproducing the experiment

If you want to reproduce the experiment in our paper, do the following

```
cd ./learned_index
cmake .
make
./learned_index
```

## Using CARMI

If you want to use only CARMI as an index, then you only need to include the header file respectively (Please keep your code file and *include* folder under the same directory level.):

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

1. **Automatic construction(CARMI)**: prepare the initial dataset, training datasets (historical access and insertion queries), and then create a CARMI object, and the algorithm will automatically build the index. Then you can perform *Find*, *Insert*, *Delete*, *Update*, *Range Scan* operations.
2. **Constructing the fixed structure in RMI and ALEX**: we have implemented the function of constructing indexes **similar (not exactly the same)** to RMI and ALEX during initialize the CARMI.

**Main functions:**

1. **Find**: find the corresponding record of the given key, return the iterator

```[C++]
CARMI<KeyType, ValueType>::iterator CARMI<KeyType, ValueType>::Find(double key);
```

2. **Insert**: insert a data point

```[C++]
bool CARMI<KeyType, ValueType>::Insert(DataType data);
```

3. **Update**: update a record ( find the record according to given key, then update the value)

```[C++]
bool CARMI<KeyType, ValueType>::Update(DataType data);
```

4. **Delete**: delete the record of the given key

```[C++]
bool CARMI<KeyType, ValueType>::Delete(double key);
```

## File structure of CARMI

In this project, we include the CARMI header files, the source code of the experimental part and the baseline. The description of each file in CARMI's header file is as follows:

- **include**
  - baseNode.h  *( the union structure of all nodes )*
  - carmi.h  *( the implementation class of CARMI )*
  - carmi_common.h  *( the CARMI class for common use )*
  - carmi_external.h  *( the CARMI class for external situation )*
  - **construct**  *( files used to construct the index )*
    - construction.h *( the main function of our algorithm )*
    - choose_root.h *( choose the optimal root node from four root nodes )*
    - construct_root.h *( use the optimal root node to construct child nodes )*
    - dp.h *( the main dynamic programming algorithm )*
    - dp_inner.h *( use DP to construct inner node )*
    - dp_leaf.h *( use DP to construct leaf node )*
    - greedy.h *( the greedy node selection algorithm )*
    - structures.h *( the structures of CARMI )*
    - store_node.h *( use the optimal setting to construct a new node )*
  - **dataManager**  *( manage the two main arrays )*
    - child_array.h
    - datapoint.h
    - empty_block.h
  - **nodes**   *( all nodes we have implemented )*
    - **rootNode**
      - root_nodes.h  *( the classes of root nodes )*
      - **trainModel** *( models used to train the root nodes )*
        - linear_regression.h
        - piecewiseLR.h
        - histogram.h
        - binary_search_model.h
        - candidate_plr.h *( for piecewiseLR )*
    - **innerNode**
      - inner_nodes.h  *( the classes of inner nodes )*
      - lr.h
      - plr.h
      - his.h
      - bin.h
    - **leafNode**
      - leaf_nodes.h  *( the classes of leaf nodes )*
      - array_type.h
      - ga_type.h
      - external_array_type.h
  - **func**  *( public functions )*
    - find_function.h
    - insert_function.h
    - delete_function.h
    - update_function.h
    - inlineFunction.h
    - calculate_space.h
    - print_structure.h

## Dependencies

This code is based on C++.

Main dependencies is [**STX-btree**](https://github.com/bingmann/stx-btree) and [**ART Tree**](https://github.com/armon/libart).
