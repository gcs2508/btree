cmake_minimum_required(VERSION 3.4)
project(btree)

add_compile_options(-O2)

include_directories(voxel_filter rw_lock)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(btree BehaviorNode.cpp main.cpp BasePid.cpp voxel_filter/voxel_filter.cpp rw_lock/RWLock.cpp)
target_link_libraries(btree m pthread)