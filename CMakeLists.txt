cmake_minimum_required(VERSION 3.0)
project(learned_index)
set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wignored-attributes")


file(GLOB cpp_btree_sources cpp-btree/*.h)
add_library(cpp_btree STATIC ${cpp_btree_sources})
target_include_directories(cpp_btree PUBLIC cpp_btree)
set_target_properties(cpp_btree PROPERTIES LINKER_LANGUAGE CXX)

# target_include_directories(learned_index PUBLIC ${EIGEN3_INCLUDE_DIR})
# INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})
add_executable(learned_index src/main.cpp)
target_link_libraries(learned_index cpp_btree)

# # set gprof
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
# SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
