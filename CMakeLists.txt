cmake_minimum_required(VERSION 3.10)
project(DataCleaningToolkit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_definitions(-DASIO_STANDALONE)

find_package(Boost REQUIRED)

# i need to include these else it breaks
include_directories(include)

# these will be the source files
set(SOURCES src/main.cpp src/algorithms.cpp)

# this will be the executable
add_executable(Toolkit ${SOURCES})

target_link_libraries(Toolkit ${Boost_LIBRARIES})

# Windows socket library
if(WIN32)
    target_link_libraries(Toolkit ws2_32 wsock32)
endif()