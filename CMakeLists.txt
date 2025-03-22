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

if (NOT ${spdlog_FOUND})
    message(STATUS "${Yellow}spdlog not found, fetching source${ColorReset}")

    ExternalProject_Add(spdlog
        PREFIX          spdlog
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
    set(SPDLOG_LIBRARIES "${DEPS_INSTALL_PREFIX}/lib/libspdlog${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(SPDLOG_INCLUDE_DIRS "${DEPS_INSTALL_PREFIX}/include/spdlog")
    set(spdlog_FOUND TRUE)
    set(SPDLOG_TARGET spdlog)
endif()

add_executable(huffman
    src/huffman.cpp
)

add_executable(adaptive_huffman
    src/adaptive_huffman.cpp
)

# add_dependencies(huffman
#     spdlog
# )
# 
# add_dependencies(adaptive_huffman
#     spdlog
# )

# target_include_directories(huffman
#     PRIVATE ${SPDLOG_INCLUDE_DIRS}
# )

target_link_libraries(huffman
    PRIVATE spdlog::spdlog
)

# target_include_directories(adaptive_huffman
#     PRIVATE ${SPDLOG_INCLUDE_DIRS}
# )

target_link_libraries(adaptive_huffman
    PRIVATE spdlog::spdlog
)

# target_link_libraries(huffman PRIVATE spdlog::spdlog_header_only)
# target_link_libraries(adaptive_huffman PRIVATE spdlog::spdlog_header_only)

# include(FetchContent)
# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY  https://github.com/google/googletest.git
#     GIT_TAG         2b6b042a77446ff322cd7522ca068d9f2a21c1d1
# )
# 
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)
# 
# enable_testing()
# 
# add_executable(misc_test
#     tests/misc_test.cpp
#     src/misc.cpp
# )
# add_dependencies(misc_test
#     Armadillo
# )
# target_link_libraries(misc_test
#     GTest::gtest_main
# )
# target_include_directories(misc_test
#     PRIVATE ${ARMADILLO_INCLUDE_DIRS}
# )
# 
# include(GoogleTest)
# gtest_discover_tests(misc_test)
# 
