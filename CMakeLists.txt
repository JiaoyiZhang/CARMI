# # macOS
# cmake_minimum_required(VERSION 3.0)
# project(learned_index)
# set(CMAKE_CXX_STANDARD 11)
# file(GLOB cpp_btree_sources cpp-btree/*.h)
# include_directories("${cpp_btree_sources}/lib")
# add_executable(learned_index src/test.cpp)


# Ubuntu

cmake_minimum_required(VERSION 3.0)
project(learned_index)
set(CMAKE_CXX_STANDARD 11)

file(GLOB stx_btree_sources stx_btree/*.h)
add_library(stx_btree STATIC ${stx_btree_sources})
target_include_directories(stx_btree PUBLIC stx_btree)
set_target_properties(stx_btree PROPERTIES LINKER_LANGUAGE CXX)

add_executable(learned_index src/test.cpp)
target_link_libraries(learned_index stx_btree)