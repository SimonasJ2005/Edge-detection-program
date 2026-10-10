cmake_minimum_required(VERSION 3.10)
project(EdgeDetection CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

set(COMMON_SOURCES src/image.cpp src/convolution.cpp src/convolution_parallel.cpp)

add_executable(edge src/main.cpp ${COMMON_SOURCES})
add_executable(benchmark src/benchmark.cpp ${COMMON_SOURCES})

foreach(target edge benchmark)
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_link_libraries(${target} PRIVATE Threads::Threads)
    if(MSVC)
        target_compile_options(${target} PRIVATE /utf-8)
        target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endforeach()