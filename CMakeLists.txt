cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(robot_farmer CXX)
add_executable(robot_farmer main.cpp)
target_compile_features(robot_farmer PRIVATE cxx_range_for)
