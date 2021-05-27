#pragma once

// Сторонние библиотеки
#include <slang.h>
#include <slang-com-ptr.h>

// Стандартные библиотеки
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>

class Shaders {
 public:
  typedef Shaders* Manager;

  typedef struct shader_t {
    std::string name;            // Полный путь до шейдера, название
    std::string entryPointName;  // Вход в шейдер (= main)
    std::vector<char> code;      // SPIR-V

    // Стадия конвейера, на которой будет выполнен шейдер
    SlangStage stage = SLANG_STAGE_NONE;

    slang::ShaderReflection* slangReflection;
    SlangCompileRequest* slangRequest;
  } * Instance;

 private:
  SlangSession* slangSession = nullptr;
  std::vector<Instance> handlers;
  Instance compileShader(Instance);

 public:
  Shaders();
  ~Shaders();

  Instance loadShader(const std::string& name, const std::string& entryPointName, SlangStage);
  void reloadAllShaders();
};
