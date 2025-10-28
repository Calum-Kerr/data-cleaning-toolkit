cmake_minimum_required(VERSION 3.10)
project(DataCleaningToolkit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# i need to include these else it breaks
include_directories(include)

# these will be the source files
set(SOURCES src/main.cpp, src/algorithms.cpp)

# this will be the executable
add_executable(Toolkit ${SOURCES})