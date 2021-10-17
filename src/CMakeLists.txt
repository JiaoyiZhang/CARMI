
# Ubuntu

cmake_minimum_required(VERSION 3.0)
project(CARMI)
set(CMAKE_CXX_STANDARD 17)

aux_source_directory(experiment/workload source_list_workload)

add_executable(CARMI experiment.cpp experiment/core.cpp experiment/main_experiment.cpp ${source_list_workload} )
target_link_libraries(CARMI)