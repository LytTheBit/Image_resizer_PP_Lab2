cmake_minimum_required(VERSION 3.20)
project(Image_resizer_PP_Lab2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type (useful in CLion if not set)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(Image_resizer_PP_Lab2
        src/main.cpp
        src/io.cpp
        src/resize_sequential.cpp
        src/resize_openmp.cpp
        src/scaling_attacks.cpp
        src/benchmark.cpp
        src/util.cpp
        src/cli.cpp
)

target_include_directories(Image_resizer_PP_Lab2 PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Warnings (adjust if you use MSVC/MinGW/Clang)
if (MSVC)
    target_compile_options(Image_resizer_PP_Lab2 PRIVATE /W4)
else()
    target_compile_options(Image_resizer_PP_Lab2 PRIVATE -Wall -Wextra -Wpedantic)
endif()

# OpenMP (required if you compile resize_openmp.cpp; otherwise you can make it optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Image_resizer_PP_Lab2 PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(Image_resizer_PP_Lab2 PRIVATE HAVE_OPENMP=1)
else()
    target_compile_definitions(Image_resizer_PP_Lab2 PRIVATE HAVE_OPENMP=0)
    message(WARNING "OpenMP not found: parallel backend will be unavailable.")
endif()

# Reasonable optimization flags for Release in GCC/Clang
if (NOT MSVC)
    target_compile_options(Image_resizer_PP_Lab2 PRIVATE
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-DNDEBUG>
    )
endif()
