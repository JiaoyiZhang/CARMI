cmake_minimum_required(VERSION 3.0)
project(unit_test)
set(CMAKE_CXX_STANDARD 11)

file(GLOB cpp_btree_sources cpp-btree/*.h)
add_library(cpp_btree STATIC ${cpp_btree_sources})
target_include_directories(cpp_btree PUBLIC cpp_btree)
set_target_properties(cpp_btree PROPERTIES LINKER_LANGUAGE CXX)

# innerNode
# add_executable(unit_test ./innerNodeTest/linear_regression_test.cpp)
# add_executable(unit_test ./innerNodeTest/network_test.cpp)
# add_executable(unit_test ./innerNodeTest/histogram_test.cpp)
# add_executable(unit_test ./innerNodeTest/binary_search_test.cpp)

# leafNode
# add_executable(unit_test ./leafNodeTest/array_test.cpp)
# add_executable(unit_test ./leafNodeTest/gapped_array_test.cpp)

# trainModel
# add_executable(unit_test ./modelTest/lr_test.cpp)
# add_executable(unit_test ./modelTest/nn_test.cpp)
# add_executable(unit_test ./modelTest/his_test.cpp)
add_executable(unit_test ./modelTest/bin_test.cpp)

target_link_libraries(unit_test cpp_btree gtest_main gtest pthread)