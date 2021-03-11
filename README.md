# A simple implementation of CARMI

This is a simple implementation of our paper: **CARMI: A Cache-Aware Learned Index with a Cost-based Construction Algorithm**.

## Building

If you want to run the program, do the following

```
cd ./learned_index
cmake .
make
./learned_index
```

### Tips

We are optimizing our code style. If you need to run the project, you need to wait for us to complete the optimization or run the version when the paper was submiited.


## Instructions

**Method of constructing an index:**

1. **Automatic construction(CARMI)**: prepare the initial dataset, training datasets (historical access and insertion queries), and then create a CARMI object, and the algorithm will automatically build the index. Then you can perform *Find*, *Insert*, *Delete*, *Update*, *Range Scan* operations.
2. **Constructing the fixed structure in RMI and ALEX**: we have implemented the function of constructing indexes **similar (not exactly the same)** to RMI and ALEX during initialize the CARMI.

## File structure of CARMI

We divided the source code into four parts: construct, dataManager, func, nodes and some shared files. The discription of each file is introduced below.

- **src**
  - baseNode.h  *( the union structure of all nodes )*
  - carmi.h  *( the CARMI class )*
  - **construct**  *( files used to construct the index )*
    - construction.h *( the main function of our algorithm )*
    - choose_root.h *( choose the optimal root node from four root nodes )*
    - construct_root.h *( use the optimal root node to construct child nodes )*
    - dp.h *( the main dynamic programming algorithm )*
    - dp_inner.h *( use DP to construct inner node )*
    - dp_leaf.h *( use DP to construct leaf node )*
    - greedy.h *( the greedy node selection algorithm )*
    - node_cost_struct.h *( the structure of NodeCost )*
    - params_struct.h *( the structure of ParamStruct )*
    - root_struct.h *( the structure of RootStruct )*
    - store_node.h *( use the optimal setting to construct a new node )*
    - update_leaf.h *( connect leaf nodes for range scan )*
  - **dataManager**  *( manage the two main arrays )*
    - child_array.h 
    - datapoint.h
    - empty_block.h
  - **nodes**   *( all nodes we have implemented )*
    - **rootNode**
      - lr_type.h
      - plr_type.h
      - his_type.h
      - bin_type.h
      - **trainModel** *( models used to train the root nodes )*
        - linear_regression.h
        - piecewiseLR.h
        - histogram.h
        - binary_search_model.h
    - **innerNode**
      - lr.h
      - plr.h
      - his.h
      - bin.h
    - **leafNode**
      - array.h
      - ga.h
      - ycsb_leaf.h
  - **func**  *( public functions )*
    - find_function.h
    - inesrt_function.h
    - delete_function.h
    - update_function.h
    - rangescan_function.h
    - calculate_space.h
    - print_structure.h

## Dependencies

This code is based on C++.

Main dependencies is [**STX-btree**](github.com/bingmann/stx-btree) and [**ART Tree**](https://github.com/armon/libart).
