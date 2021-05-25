#pragma once

// Сторонние библиотеки
#include <slang.h>
#include <slang-com-ptr.h>

// Стандартные библиотеки
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>

typedef class Shaders* ShadersManager;

class Shaders {
 private:
  // Дискриптор шейдера
  struct shader_t {
    std::string name;            // Полный путь до шейдера, название
    std::string entryPointName;  // Вход в шейдер (= main)
    std::vector<char> code;      // SPIR-V

    // Стадия конвейера, на которой будет выполнен шейдер
    SlangStage stage = SLANG_STAGE_NONE;

    slang::ShaderReflection* slangReflection;
    SlangCompileRequest* slangRequest;
  };

  std::vector<shader_t> handlers;
  SlangSession* slangSession = nullptr;

 public:
  Shaders();
  ~Shaders();

  uint32_t loadShader(const char* name, const char* entryPointName, SlangStage);
  shader_t compileShader(const char* name, const char* entryPointName, SlangStage);
  bool getShaderCode(uint32_t id, const char*& code, uint32_t& size);
  void reloadAllShaders();
};
