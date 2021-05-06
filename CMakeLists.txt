cmake_minimum_required(VERSION 3.20)
project(nevk)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set output directory for executable files
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

# Vulkan
find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

