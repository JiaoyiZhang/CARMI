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
    - weightInitializer.h (Initialize the weight parameters of the network)
    - rmi.h (The realization of the main algorithm of RMI model)
    - optimizer.h
    - net.h (The class of the net)
    - main.cpp 
    - loss.h
    - layer.h
- cpp-btree (The dependency)
- README.md
- CMakeLists.txt


## Dependencies
This code is based on C++.

Main dependencies are **Eigen 3.3.4** and **cpp-btree**.