cmake_minimum_required(VERSION 3.20)
project(nevk)

#================================================================
# Common variables and options

# Set required C++ standard
set(CMAKE_CXX_STANDARD 20)

# Set output directory for executable files
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

#================================================================
# Requirements

find_package(Vulkan REQUIRED) # There should be Vulkan SDK on local computer

#================================================================
# Internal libraries

# ENGINE
set(LIBRARY_ENGINE_NAME engine)
set(LIBRARY_ENGINE_PATH src/engine)
set(LIBRARY_ENGINE_SOURCES
    ${LIBRARY_ENGINE_PATH}/core/validation.h
    ${LIBRARY_ENGINE_PATH}/core/core.h
    ${LIBRARY_ENGINE_PATH}/core/core.cpp

    ${LIBRARY_ENGINE_PATH}/engine.h
    ${LIBRARY_ENGINE_PATH}/engine.cpp
)
add_library(${LIBRARY_ENGINE_NAME} OBJECT ${LIBRARY_ENGINE_SOURCES})
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC ${LIBRARY_ENGINE_PATH})
target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC Vulkan::Vulkan)

#================================================================
# External libraries

# GLFW
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC external/glfw/include)
if (MSVC)
    target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/glfw3.lib")
elseif (MINGW)
    target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/libglfw3.a")
elseif (APPLE)
    find_package(glfw3 3.3 REQUIRED)
    include_directories(${glfw3_INCLUDE_DIRS})
    target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC glfw)
endif ()


#================================================================
# Application

add_executable(${PROJECT_NAME} src/app/app.cpp src/app/app.h)
target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${LIBRARY_ENGINE_NAME}
)
