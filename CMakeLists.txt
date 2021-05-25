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

# SHADERS
set(LIBRARY_SHADERS_NAME shaders)
set(LIBRARY_SHADERS_PATH src/engine/render/shaders)
set(LIBRARY_SHADERS_SOURCES
    ${LIBRARY_SHADERS_PATH}/shaders.h
    ${LIBRARY_SHADERS_PATH}/shaders.cpp
)
add_library(${LIBRARY_SHADERS_NAME} OBJECT ${LIBRARY_SHADERS_SOURCES})
target_include_directories(${LIBRARY_SHADERS_NAME} PUBLIC ${LIBRARY_SHADERS_PATH})

# RENDER
set(LIBRARY_RENDER_NAME render)
set(LIBRARY_RENDER_PATH src/engine/render)
set(LIBRARY_RENDER_SOURCES
    ${LIBRARY_RENDER_PATH}/commands/commands.h
    ${LIBRARY_RENDER_PATH}/commands/commands.cpp
    ${LIBRARY_RENDER_PATH}/pass/pass.h
    ${LIBRARY_RENDER_PATH}/pass/pass.cpp
    ${LIBRARY_RENDER_PATH}/pass/geometry.h
    ${LIBRARY_RENDER_PATH}/pass/geometry.cpp
)
add_library(${LIBRARY_RENDER_NAME} OBJECT ${LIBRARY_RENDER_SOURCES})
target_include_directories(${LIBRARY_RENDER_NAME} PUBLIC ${LIBRARY_RENDER_PATH})
target_link_libraries(${LIBRARY_RENDER_NAME} PUBLIC 
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_RESOURCES_NAME}
    ${LIBRARY_SHADERS_NAME}
)

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

# GLFW -> Engine
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC external/glfw/include) 
target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/glfw/glfw3.lib")

# SLANG -> ShaderManager
target_include_directories(${LIBRARY_SHADERS_NAME} PUBLIC external/slang/include)
target_link_libraries(${LIBRARY_SHADERS_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/slang/slang.lib")

#================================================================
# Приложение

add_executable(${PROJECT_NAME} src/app/app.cpp src/app/app.h)
target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${LIBRARY_ENGINE_NAME}
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_RESOURCES_NAME}
    ${LIBRARY_RENDER_NAME}
    ${LIBRARY_SHADERS_NAME}
)

# Копирование шейдеров в рабочую директорию
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_SOURCE_DIR}/src/shaders"
        "${OUTPUT_DIRECTORY}/shaders"
        )

# Копирование библиотеки SLANG в рабочую директорию
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/external/slang/slang.dll"
        "${OUTPUT_DIRECTORY}"
        )
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/external/slang/slang-glslang.dll"
        "${OUTPUT_DIRECTORY}"
        )
