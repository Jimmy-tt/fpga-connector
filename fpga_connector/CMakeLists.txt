cmake_minimum_required(VERSION 3.9)
project(fpga_connector)

set(CMAKE_CXX_STANDARD 11)

add_executable(fpga_connector main.cpp common.h CreateTruthTable.cpp CreateTruthTable.h ConditionTree.h ConditionTree.cpp ConditionHandle.cpp ConditionHandle.h ColumnHandle.cpp ColumnHandle.h ComplexCondHandle.cpp ComplexCondHandle.h)