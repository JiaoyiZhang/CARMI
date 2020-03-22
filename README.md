# A simple implementation of Learned Index

This is a simple implementation of [The Case for Learned Index Structures](https://arxiv.org/pdf/1712.01208.pdf) and [ALEX: An Updatable Adaptive Learned Index](https://arxiv.org/abs/1905.08898)

## Building
If you want to run the program, do the following 
```
cd ./learned_index
cmake .
make
./learned_index
```

## File structure

- **src**
    - **innerNode**  *( Upper models to choose from )*
        - adaptiveRMI.h
        - scaleModel.h
        - staticRMI.h
    - **leafNode**   *( Lower models to choose from )*
        - gappedNode.h
        - normalNode.h
    - **trainModel**  *( Alternative training methods )*
        - lr.h
        - nn.h
    - params.h
    - testMain.cpp
    - userMain.cpp 
- cpp-btree (The dependency)
- README.md
- CMakeLists.txt


## Dependencies
This code is based on C++.

Main dependencies is **cpp-btree**.