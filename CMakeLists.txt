cmake_minimum_required(VERSION 3.20)
project(nevk)

#===============================================================================================
# Переменные общего назначения

# Требуемый стандарт C++
set(CMAKE_CXX_STANDARD 20)

# Конечная директория для исполняемых файлов
set(OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})

#===============================================================================================
# Требования

if (NOT DEFINED VULKAN_SDK)
    find_package(Vulkan REQUIRED) # Необходим Vulkan SDK на локальном ПК
endif()

#===============================================================================================
# Внутренние библиотеки

###############
# WINDOW
###############
set(LIBRARY_WINDOW_NAME window)
set(LIBRARY_WINDOW_PATH src/window)
set(LIBRARY_WINDOW_SOURCES
    ${LIBRARY_WINDOW_PATH}/window.h
    ${LIBRARY_WINDOW_PATH}/window.cpp
)
add_library(${LIBRARY_WINDOW_NAME} OBJECT ${LIBRARY_WINDOW_SOURCES})
target_include_directories(${LIBRARY_WINDOW_NAME} PUBLIC ${LIBRARY_WINDOW_PATH})

###############
# CORE
###############
set(LIBRARY_CORE_NAME core)
set(LIBRARY_CORE_PATH src/engine/core)
set(LIBRARY_CORE_SOURCES
    ${LIBRARY_CORE_PATH}/core.h
    ${LIBRARY_CORE_PATH}/core.cpp
    ${LIBRARY_CORE_PATH}/validation.h

    ${LIBRARY_CORE_PATH}/resources/resources.h
    ${LIBRARY_CORE_PATH}/resources/resources.cpp

    ${LIBRARY_CORE_PATH}/commands/commands.h
    ${LIBRARY_CORE_PATH}/commands/commands.cpp
)
add_library(${LIBRARY_CORE_NAME} OBJECT ${LIBRARY_CORE_SOURCES})
target_include_directories(${LIBRARY_CORE_NAME} PUBLIC ${LIBRARY_CORE_PATH})

###############
# SCENE
###############
set(LIBRARY_SCENE_NAME scene)
set(LIBRARY_SCENE_PATH src/engine/scene)
set(LIBRARY_SCENE_SOURCES
    ${LIBRARY_SCENE_PATH}/scene.h
    ${LIBRARY_SCENE_PATH}/scene.cpp

    ${LIBRARY_SCENE_PATH}/resources/textures.h
    ${LIBRARY_SCENE_PATH}/resources/textures.cpp
    ${LIBRARY_SCENE_PATH}/resources/models.h
    ${LIBRARY_SCENE_PATH}/resources/models.cpp

    ${LIBRARY_SCENE_PATH}/objects/object.h
    ${LIBRARY_SCENE_PATH}/objects/object.cpp
    ${LIBRARY_SCENE_PATH}/objects/camera.h
    ${LIBRARY_SCENE_PATH}/objects/camera.cpp
)
add_library(${LIBRARY_SCENE_NAME} OBJECT ${LIBRARY_SCENE_SOURCES})
target_include_directories(${LIBRARY_SCENE_NAME} PUBLIC ${LIBRARY_SCENE_PATH})
target_link_libraries(${LIBRARY_SCENE_NAME} PUBLIC ${LIBRARY_CORE_NAME})

###############
# SHADERS
###############
set(LIBRARY_SHADERS_NAME shaders)
set(LIBRARY_SHADERS_PATH src/engine/render/shaders)
set(LIBRARY_SHADERS_SOURCES
    ${LIBRARY_SHADERS_PATH}/shaders.h
    ${LIBRARY_SHADERS_PATH}/shaders.cpp
)
add_library(${LIBRARY_SHADERS_NAME} OBJECT ${LIBRARY_SHADERS_SOURCES})
target_include_directories(${LIBRARY_SHADERS_NAME} PUBLIC ${LIBRARY_SHADERS_PATH})
target_link_libraries(${LIBRARY_SHADERS_NAME} PUBLIC ${LIBRARY_CORE_NAME})

###############
# RENDER
###############
set(LIBRARY_RENDER_NAME render)
set(LIBRARY_RENDER_PATH src/engine/render)
set(LIBRARY_RENDER_SOURCES
    ${LIBRARY_RENDER_PATH}/render.h
    ${LIBRARY_RENDER_PATH}/render.cpp

    ${LIBRARY_RENDER_PATH}/frames/frames.h
    ${LIBRARY_RENDER_PATH}/frames/frames.cpp

    ${LIBRARY_RENDER_PATH}/passes/pass.h
    ${LIBRARY_RENDER_PATH}/passes/pass.cpp

    ${LIBRARY_RENDER_PATH}/passes/graphics/graphics.h
    ${LIBRARY_RENDER_PATH}/passes/graphics/graphics.cpp
    ${LIBRARY_RENDER_PATH}/passes/graphics/geometry.h
    ${LIBRARY_RENDER_PATH}/passes/graphics/geometry.cpp

    ${LIBRARY_RENDER_PATH}/passes/graphics/postprocessing/fullscreen.h
    ${LIBRARY_RENDER_PATH}/passes/graphics/postprocessing/fullscreen.cpp
    ${LIBRARY_RENDER_PATH}/passes/graphics/postprocessing/gui.h
    ${LIBRARY_RENDER_PATH}/passes/graphics/postprocessing/gui.cpp
)
add_library(${LIBRARY_RENDER_NAME} OBJECT ${LIBRARY_RENDER_SOURCES})
target_include_directories(${LIBRARY_RENDER_NAME} PUBLIC ${LIBRARY_RENDER_PATH})
target_link_libraries(${LIBRARY_RENDER_NAME} PUBLIC 
    ${LIBRARY_WINDOW_NAME}
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_SCENE_NAME}
    ${LIBRARY_SHADERS_NAME}
)

###############
# ENGINE
###############
set(LIBRARY_ENGINE_NAME engine)
set(LIBRARY_ENGINE_PATH src/engine)
set(LIBRARY_ENGINE_SOURCES
    ${LIBRARY_ENGINE_PATH}/engine.h
    ${LIBRARY_ENGINE_PATH}/engine.cpp
)
add_library(${LIBRARY_ENGINE_NAME} OBJECT ${LIBRARY_ENGINE_SOURCES})
target_include_directories(${LIBRARY_ENGINE_NAME} PUBLIC ${LIBRARY_ENGINE_PATH})
target_link_libraries(${LIBRARY_ENGINE_NAME} PUBLIC 
    ${LIBRARY_WINDOW_NAME}
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_SCENE_NAME}
    ${LIBRARY_RENDER_NAME}
)

#===============================================================================================
# Сторонние библиотеки

# Vulkan (Графический драйвер)
if (NOT DEFINED VULKAN_SDK)
    target_link_libraries(${LIBRARY_CORE_NAME} PUBLIC Vulkan::Vulkan)
else()
    target_link_libraries(${LIBRARY_CORE_NAME} PUBLIC "${VULKAN_SDK}/lib/vulkan-1.lib")
    target_include_directories(${LIBRARY_CORE_NAME} PUBLIC "${VULKAN_SDK}/include") 
    message(STATUS "VULKAN LIBRARY: ${VULKAN_SDK}/lib/vulkan-1.lib")
    message(STATUS "VULKAN INCLUDE: ${VULKAN_SDK}/include")
endif()

# GLFW (Оконный менеджер)
target_include_directories(${LIBRARY_WINDOW_NAME} PUBLIC external/glfw/include) 
target_link_libraries(${LIBRARY_WINDOW_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/glfw/glfw3.lib" ${LIBRARY_CORE_NAME})

# SLANG (Компиляция шейдеров)
target_include_directories(${LIBRARY_SHADERS_NAME} PUBLIC external/slang/include)
target_link_libraries(${LIBRARY_SHADERS_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/external/slang/slang.lib")

# STB (Загрузка изображений)
target_include_directories(${LIBRARY_SCENE_NAME} PUBLIC external/stb)

# TOL (Загрузка объектов)
target_include_directories(${LIBRARY_SCENE_NAME} PUBLIC external/tol)

# GLM (Работа с матрицами)
target_include_directories(${LIBRARY_SCENE_NAME} PUBLIC external/glm)

# ImGUI (Пользовательский интерфейс)
set(LIBRARY_IMGUI_NAME imgui)
set(LIBRARY_IMGUI_PATH external/imgui)
set(LIBRARY_IMGUI_SOURCES
    ${LIBRARY_IMGUI_PATH}/imconfig.h
    ${LIBRARY_IMGUI_PATH}/imgui.h
    ${LIBRARY_IMGUI_PATH}/imgui_internal.h
    ${LIBRARY_IMGUI_PATH}/imgui.cpp
    ${LIBRARY_IMGUI_PATH}/imgui_tables.cpp
    ${LIBRARY_IMGUI_PATH}/imgui_draw.cpp
    ${LIBRARY_IMGUI_PATH}/imgui_widgets.cpp
    ${LIBRARY_IMGUI_PATH}/backends/imgui_impl_vulkan.cpp
    ${LIBRARY_IMGUI_PATH}/backends/imgui_impl_glfw.cpp
)
add_library(${LIBRARY_IMGUI_NAME} OBJECT ${LIBRARY_IMGUI_SOURCES})
target_include_directories(${LIBRARY_IMGUI_NAME} PUBLIC ${LIBRARY_IMGUI_PATH})
target_link_libraries(${LIBRARY_IMGUI_NAME} PUBLIC ${LIBRARY_WINDOW_NAME} ${LIBRARY_CORE_NAME})

target_link_libraries(${LIBRARY_RENDER_NAME} PUBLIC ${LIBRARY_IMGUI_NAME})

#===============================================================================================
# Приложение

add_executable(${PROJECT_NAME} src/app/app.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC src/)
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${LIBRARY_WINDOW_NAME}
    ${LIBRARY_CORE_NAME}
    ${LIBRARY_SCENE_NAME}
    ${LIBRARY_SHADERS_NAME}
    ${LIBRARY_RENDER_NAME}
    ${LIBRARY_ENGINE_NAME}

    ${LIBRARY_IMGUI_NAME}
)

# Копирование шейдеров в рабочую директорию
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_SOURCE_DIR}/src/shaders"
    "${OUTPUT_DIRECTORY}/shaders"
)

# Копирование моделей и текстур в рабочую директорию
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
    "${CMAKE_SOURCE_DIR}/misc"
    "${OUTPUT_DIRECTORY}/misc"
)

# Копирование динамической библиотеки SLANG в рабочую директорию
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
