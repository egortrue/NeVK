#pragma once

// Сторонние библиотеки
#include <slang.h>
#include <slang-com-ptr.h>

// Внутренние библиотеки
#include "core.h"

// Стандартные библиотеки
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <utility>
#include <unordered_map>

class Shaders {
 public:
  typedef Shaders* Manager;
  Core::Manager core;

  typedef struct shader_t {
    std::string name;
    std::string entryPoint;

    SlangStage stage;
    std::vector<char> code;
    VkShaderModule module;
  } * Instance;

 private:
  SlangSession* slangSession = nullptr;
  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

 public:
  explicit Shaders(Core::Manager core);
  ~Shaders();
  void reload();

  Instance loadShader(const std::string& name, const std::string& entryPoint, SlangStage);
  void reloadShader(const std::string& name, const std::string& entryPoint);
  void destroyShader(const std::string& name, const std::string& entryPoint);

 private:
  void compileShader(Instance, SlangStage);
  void destroyShader(Instance);
};
