cmake_minimum_required(VERSION 3.18)

project(Information_Theory_Adaptive_Huffman CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
    string(ASCII 27 Esc)
    set(ColorReset  "${Esc}[m")
    set(Green       "${Esc}[32m")
    set(Yellow      "${Esc}[33m")
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(DEPS_PREFIX ${CMAKE_BINARY_DIR}/_deps)
set(DEPS_INSTALL_PREFIX ${DEPS_PREFIX}/install)

list(APPEND CMAKE_PREFIX_PATH "${DEPS_INSTALL_PREFIX}")
list(APPEND CMAKE_BUILD_RPATH "${DEPS_INSTALL_PREFIX}/lib")

include(ExternalProject)

find_package(spdlog QUIET)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()
find_package(Boost QUIET)

if (NOT ${spdlog_FOUND})
    message(STATUS "${Yellow}spdlog not found, fetching source${ColorReset}")

    ExternalProject_Add(spdlog
        GIT_REPOSITORY  https://github.com/gabime/spdlog.git
        GIT_TAG         v1.15.1
        UPDATE_COMMAND  ""
        PREFIX          ${DEPS_PREFIX}
        CMAKE_ARGS      
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_PREFIX}
            -DBUILD_SHARED_LIBS=ON
    )
    add_library(spdlog_interface INTERFACE)
    add_dependencies(spdlog_interface spdlog)
    target_include_directories(spdlog_interface INTERFACE "${DEPS_INSTALL_PREFIX}/include")
    target_link_directories(spdlog_interface INTERFACE "${DEPS_INSTALL_PREFIX}/lib")
    add_library(spdlog::spdlog ALIAS spdlog_interface)
endif()

if (NOT ${Boost_FOUND})
    message(STATUS "${Yellow}boost library not found, fetching source${ColorReset}")

    ExternalProject_Add(Boost
        GIT_REPOSITORY  https://github.com/boostorg/boost.git
        GIT_TAG         boost-1.87.0
        UPDATE_COMMAND  ""
        PREFIX          ${DEPS_PREFIX}
        CMAKE_ARGS      
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_INSTALL_PREFIX=${DEPS_INSTALL_PREFIX}
            -DBUILD_SHARED_LIBS=ON
    )

    add_library(boost_interface INTERFACE)
    add_dependencies(boost_interface Boost)
    target_include_directories(boost_interface INTERFACE "${DEPS_INSTALL_PREFIX}/include")
    target_link_directories(boost_interface INTERFACE "${DEPS_INSTALL_PREFIX}/lib")
    add_library(Boost::boost ALIAS boost_interface)
endif()

add_executable(huffman
    src/huffman.cpp
    src/argparse.cpp
    src/dataloader.cpp
    src/coder.cpp
    src/utils.cpp
)

add_executable(adaptive_huffman
    src/adaptive_huffman.cpp
    src/coder.cpp
    src/argparse.cpp
    src/dataloader.cpp
    src/tree.cpp
    src/node.cpp
    src/utils.cpp
)

target_link_libraries(huffman
    PRIVATE spdlog::spdlog
)

target_link_libraries(adaptive_huffman
    PRIVATE spdlog::spdlog
)

add_dependencies(huffman spdlog::spdlog Boost::boost)
add_dependencies(adaptive_huffman spdlog::spdlog Boost::boost)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         2ae29b52fdff88c52fef655fa0d245fc514ca35b
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(node_test
    tests/node_test.cpp
    src/node.cpp
)

add_executable(tree_test
    tests/tree_test.cpp
    src/dataloader.cpp
    src/node.cpp
    src/tree.cpp
)
add_executable(dataloader_test
    tests/dataloader_test.cpp
    src/dataloader.cpp
)

# for #include <gtest/gtest.h>
target_link_libraries(node_test
    GTest::gtest_main
    Boost::boost
    spdlog::spdlog
)
target_link_libraries(tree_test
    GTest::gtest_main
    Boost::boost
    spdlog::spdlog
)
target_link_libraries(dataloader_test
    GTest::gtest_main
    spdlog::spdlog
)

include(GoogleTest)
gtest_discover_tests(tree_test)

