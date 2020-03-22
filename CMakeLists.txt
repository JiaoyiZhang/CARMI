cmake_minimum_required(VERSION 3.0)
project(learned_index)
set(CMAKE_CXX_STANDARD 11)

# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wignored-attributes")

file(GLOB cpp_btree_sources cpp-btree/*.h)
add_library(cpp_btree STATIC ${cpp_btree_sources})
target_include_directories(cpp_btree PUBLIC cpp_btree)
set_target_properties(cpp_btree PROPERTIES LINKER_LANGUAGE CXX)

add_executable(learned_index src/testMain.cpp)
target_link_libraries(learned_index cpp_btree)
