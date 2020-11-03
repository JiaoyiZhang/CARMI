# A simple implementation of Learned Index

This is a simple implementation of our paper: **An Optimization-based Index Construction Strategy for Learned Index**.

## Building

If you want to run the program, do the following

```
cd ./learned_index
cmake .
make
./learned_index
```

## Instructions

**Method of constructing an index (static structure):**

1. **Manual setting**: choose one of the four inner nodes, then select the corresponding class as the leaf node, modify the parameters in param.h, and you can build an index.
2. **Automatic construction**: call the function of reconstruction.h, set the initial inner node, the program will automatically select the appropriate parameters to build the index based on the dataset, past read-write times, workload and other information.

## File structure

We divided the source code into five parts: dataset, innerNode, leafNode, trainModel and several shared files. The discription of each cpp file is introduced below.

- **src**
  - **innerNode**  *( Upper models to choose from )*
    - inner_node.h  *( basic class )*
    - lr_node.h
    - nn_node.h
    - binary_search.h
    - division_node.h
  - **leafNode**   *( Lower models to choose from )*
    - leaf_node.h  *( basic class )*
    - array.h
    - gapped_array.h
  - **trainModel**  *( Alternative training methods )*
    - model.h  *( basic class )*
    - histogram.h
    - binary_search_model.h
    - lr.h
    - nn.h
  - inner_node_creator.h  *( only used in adaptive structure )*
  - leaf_node_creator.h  *( used by the upper model to select the type of leaf node to construct )*
  - params.h  *( basic parameters for building an index )*
  - reconstruction.h
  - main.cpp  *( contains various test functions )*
  - userMain.cpp  *( contains only basic functions used by users )*
- cpp-btree *( The dependency )*
- README.md
- CMakeLists.txt

## Dependencies

This code is based on C++.

Main dependencies is [**STX-btree**](github.com/bingmann/stx-btree).
