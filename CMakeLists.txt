# # macOS
# cmake_minimum_required(VERSION 3.0)
# project(learned_index)
# set(CMAKE_CXX_STANDARD 11)
# file(GLOB cpp_btree_sources cpp-btree/*.h)
# include_directories("${cpp_btree_sources}/lib")
# add_executable(learned_index src/main.cpp)


# Ubuntu


cmake_minimum_required(VERSION 3.0)
project(learned_index)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")  # debug mode

file(GLOB stx_btree_sources stx_btree/*.h)
add_library(stx_btree STATIC ${stx_btree_sources})
target_include_directories(stx_btree PUBLIC stx_btree)
set_target_properties(stx_btree PROPERTIES LINKER_LANGUAGE CXX)
aux_source_directory(src/experiment/workload source_list_workload)

add_executable(learned_index src/experiment.cpp src/experiment/core.cpp src/experiment/main_experiment.cpp ${source_list_workload} )
target_link_libraries(learned_index stx_btree)