
# Ubuntu

cmake_minimum_required(VERSION 3.0)
project(CARMI)
set(CMAKE_CXX_STANDARD 17)

aux_source_directory(experiment/workload source_list_workload)
SET(CMAKE_BUILD_TYPE "Release")
# add_executable(CARMI profiler/inner_node_time.cpp)
# add_executable(CARMI profiler/leaf_node_time.cpp)
add_executable(CARMI experiment.cpp experiment/core.cpp experiment/main_experiment.cpp ${source_list_workload} )
target_link_libraries(CARMI)

## Test
# add_executable(CARMI unitTest/carmiTest/carmi_map_test.cpp)
# add_executable(CARMI unitTest/carmiTest/carmi_external_map_test.cpp)

## rootNode
# add_executable(CARMI unitTest/rootNodeTest/piecewiseLR_test.cpp)
# add_executable(CARMI unitTest/rootNodeTest/lr_test.cpp)

## innerNode
# add_executable(CARMI unitTest/innerNodeTest/linear_regression_test.cpp)
# add_executable(CARMI unitTest/innerNodeTest/piecewise_lr_test.cpp)
# add_executable(CARMI unitTest/innerNodeTest/histogram_test.cpp)
# add_executable(CARMI unitTest/innerNodeTest/binary_search_test.cpp)

## leafNode
# add_executable(CARMI unitTest/leafNodeTest/cfarray_test.cpp)
# add_executable(CARMI unitTest/leafNodeTest/external_array_test.cpp)
 
 
# target_link_libraries(CARMI gtest_main gtest pthread)