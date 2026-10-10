cmake_minimum_required(VERSION 3.10)
project(EdgeDetectionProgram CXX)

set(CMAKE_CXX_STANDART 17)
set(CMAKE_CXX_STANDART_REQUIRED ON)

# Jei vartotojas nenurodo build tipo - naudojamas release
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

add_executable(edge
    src/main.cpp
    src/convolution_parallel.cpp
    src/convolution.cpp
    src/image.cpp
)

target_link_libraries(edge PRIVATE Threads::Threads)