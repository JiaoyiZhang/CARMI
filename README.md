# A simple implementation of Learned Index

This is a simple implementation of [The Case for Learned Index Structures](https://arxiv.org/pdf/1712.01208.pdf).

## Building
If you want to run the program, do the following 
```
cd ./learned_index
cmake .
make
./learned_index
```

## File structure

- src
    - learnedIndex.h (The realization of the main algorithm of RMI model)
    - nn.h (The class of the net)
    - main.cpp 
- cpp-btree (The dependency)
- README.md
- CMakeLists.txt


## Dependencies
This code is based on C++.

Main dependencies is **cpp-btree**.