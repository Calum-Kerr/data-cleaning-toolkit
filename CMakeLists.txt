cmake_minimum_required(VERSION 3.10)
project(DataCleaningToolkit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DASIO_STANDALONE)
include_directories(include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/include)
set(SOURCES src/main.cpp src/algorithms.cpp)
add_executable(Toolkit ${SOURCES})
if(WIN32)
    target_link_libraries(Toolkit PRIVATE ws2_32 wsock32)
endif()
