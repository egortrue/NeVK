cmake_minimum_required(VERSION 3.20)
project(nevk)

#================================================================
# Переменные общего назначения

# Требуемый стандарт C++
set(CMAKE_CXX_STANDARD 20)

# Конечная директория для исполняемых файлов
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

#================================================================
# Требования

find_package(Vulkan REQUIRED) # Необходим Vulkan SDK на локальном ПК

#================================================================
# Внутренние библиотеки

# CORE
set(LIBRARY_CORE_NAME core)
set(LIBRARY_CORE_PATH src/engine/core)
set(LIBRARY_CORE_SOURCES
    ${LIBRARY_CORE_PATH}/validation.h
    ${LIBRARY_CORE_PATH}/core.h
    ${LIBRARY_CORE_PATH}/core.cpp
)
add_library(${LIBRARY_CORE_NAME} OBJECT ${LIBRARY_CORE_SOURCES})
target_include_directories(${LIBRARY_CORE_NAME} PUBLIC ${LIBRARY_CORE_PATH})
target_link_libraries(${LIBRARY_CORE_NAME} PUBLIC Vulkan::Vulkan)

# RESOURCES
set(LIBRARY_RESOURCES_NAME resources)
set(LIBRARY_RESOURCES_PATH src/engine/resources)
set(LIBRARY_RESOURCES_SOURCES
    ${LIBRARY_RESOURCES_PATH}/resources.h
    ${LIBRARY_RESOURCES_PATH}/resources.cpp
)
add_library(${LIBRARY_RESOURCES_NAME} OBJECT ${LIBRARY_RESOURCES_SOURCES})
target_include_directories(${LIBRARY_RESOURCES_NAME} PUBLIC ${LIBRARY_RESOURCES_PATH})
target_link_libraries(${LIBRARY_RESOURCES_NAME} PUBLIC ${LIBRARY_CORE_NAME})

# RENDER
set(LIBRARY_RENDER_NAME render)
set(LIBRARY_RENDER_PATH src/engine/render)
set(LIBRARY_RENDER_SOURCES
    ${LIBRARY_RENDER_PATH}/commands.h
    ${LIBRARY_RENDER_PATH}/commands.cpp
)
add_library(${LIBRARY_RENDER_NAME} OBJECT ${LIBRARY_RENDER_SOURCES})
target_include_directories(${LIBRARY_RENDER_NAME} PUBLIC ${LIBRARY_RENDER_PATH})
target_link_libraries(${LIBRARY_RENDER_NAME} PUBLIC ${LIBRARY_CORE_NAME})


# ENGINE
set(LIBRARY_ENGINE_NAME engine)
set(LIBRARY_ENGINE_PATH src/engine)
set(LIBRARY_ENGINE_SOURCES
    ${LIBRARY_ENGINE_PATH}/engine.h
    ${LIBRARY_ENGINE_PATH}/engine.cpp
)
add_library(${LIBRARY_ENGINE_NAME} OBJECT ${LIBRARY_ENGINE_SOURCES})
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC ${LIBRARY_ENGINE_PATH})
target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC 
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_RESOURCES_NAME}
    ${LIBRARY_RENDER_NAME}
)

#================================================================
# Внешние библиотеки

# GLFW
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC external/glfw/include) 
if (MSVC)
    target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC 
        "${CMAKE_SOURCE_DIR}/external/glfw/glfw3.lib")
endif ()

#================================================================
# Приложение

add_executable(${PROJECT_NAME} src/app/app.cpp src/app/app.h)
target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${LIBRARY_ENGINE_NAME}
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_RESOURCES_NAME}
    ${LIBRARY_RENDER_NAME}
)
