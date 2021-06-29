# NeVK | No Error Vulkan
<p align="center"><img src="img/error.gif" height=150/></p>
<p align="center">
    <a>
        <img src="https://img.shields.io/badge/runs%20on-C++-ffa">
    </a>
    <a href="https://vulkan.org/">
        <img src="https://img.shields.io/badge/runs%20on-Vulkan-ffa">
    </a>
    <a><img src="https://img.shields.io/badge/platform-Windows-aff"></a>
    <a><img src="https://img.shields.io/tokei/lines/github/egortrue/NeVK"></a>
    <a><img src="https://img.shields.io/github/repo-size/egortrue/NeVK"></a>
</p>
<p align="center">
    <a href="https://github.com/egortrue/NeVK/actions/workflows/build.yml">
        <img src="https://img.shields.io/github/workflow/status/egortrue/NeVK/build">
    </a>
    <a href="https://github.com/egortrue/NeVK/actions/workflows/cpplint.yml">
        <img src="https://img.shields.io/github/workflow/status/egortrue/NeVK/cpplint?label=cpplint">
    </a> 
</p>

## Описание
Целью проекта "No Error Vulkan" (сокр. NeVK) является изучение основ работы с графическим процессором на основе Vulkan API и реализация базовых компонентов видеоигр.

## Минимальные требования
- OS: Windows 10
- Graphics:  NVIDIA GeForce GTX 600, AMD Radeon HD 7700
- Дополнительный софт:
    - Git v2.32
    - CMake v3.19
    - Vulkan SDK v1.2.176
    - Microsoft Visual C++ 2019

## Установка и запуск

1) Загрузка репозитория со всеми зависимостями:  
`git clone --recursive https://github.com/egortrue/NeVK.git`
2) Сборка и компиляция проекта:  
`cd Nevk && mkdir build && cd build`  
`cmake .. && cmake --build .`
3) Запуск приложения:  
`cd bin && nevk.exe`

## Примеры
<div align="center">
    <img src="img/object1.gif" height=300/>
    <p>Простеший объект</p>
    <img src="img/object2.gif" height=300/>
    <p>Составной объект</p>
    <img src="img/scene.png" height=400/>
    <p>Высоконагруженная сцена</p>
</div>
