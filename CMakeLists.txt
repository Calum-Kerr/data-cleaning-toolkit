if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()
cmake_minimum_required(VERSION 3.10)
project(DataCleaningToolkit)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hardening flags (only for gcc/clang style toolchains; MSVC will error on these)
if(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat -Wformat-security")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack -Wl,-z,relro,-z,now")
endif()
include_directories(include)
set(SOURCES src/main.cpp src/algorithms.cpp)
add_executable(Toolkit ${SOURCES})
find_package(Threads REQUIRED)

# if vcpkg toolchain is not active, guess the usual triplet so we can still find headers
if(NOT DEFINED VCPKG_TARGET_TRIPLET OR VCPKG_TARGET_TRIPLET STREQUAL "")
    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows")
    elseif(UNIX AND NOT APPLE)
        set(VCPKG_TARGET_TRIPLET "x64-linux")
    endif()
endif()

set(_ASIO_HINT_DIRS
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
    "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include"
    "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include"
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/${VCPKG_TARGET_TRIPLET}/include"
    "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/include"
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-windows/include"
    "${CMAKE_SOURCE_DIR}/vcpkg/installed/x64-linux/include"
    "/usr/include"
)

find_path(ASIO_INCLUDE_DIR NAMES asio.hpp HINTS ${_ASIO_HINT_DIRS})
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "asio.hpp not found. make sure vcpkg installed 'asio' and cmake is using the vcpkg toolchain.")
endif()
target_include_directories(Toolkit PRIVATE ${ASIO_INCLUDE_DIR})
target_compile_definitions(Toolkit PRIVATE ASIO_STANDALONE)

if(WIN32)
    target_link_libraries(Toolkit PRIVATE ws2_32 wsock32)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(Toolkit PRIVATE pthread)
endif()
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()