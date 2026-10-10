cmake_minimum_required(VERSION 3.10)
project(EdgeDetection CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(edge
    src/main.cpp
    src/image.cpp
    src/convolution.cpp
    src/convolution_parallel.cpp)

target_compile_features(edge PRIVATE cxx_std_17)
target_link_libraries(edge PRIVATE Threads::Threads)

if(MSVC)
    target_compile_options(edge PRIVATE /utf-8)
    target_compile_definitions(edge PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()