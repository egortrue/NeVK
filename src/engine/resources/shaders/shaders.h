#pragma once

// Сторонние библиотеки
#include <slang.h>
#include <slang-com-ptr.h>

// Стандартные библиотеки
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <unordered_map>

class Shaders {
 public:
  typedef Shaders* Manager;
  typedef struct shader_t {
    // Путь до шейдера
    std::string name;

    // Вершинный шейдер
    std::string vertexName;
    std::vector<char> vertexCode;

    // Фрагментный шейдер
    std::string fragmentName;
    std::vector<char> fragmentCode;

  } * Instance;

 public:
  Shaders();
  ~Shaders();

  Instance loadShader(const std::string& name, const std::string& vertexName, const std::string& fragmentName);
  void reloadShader(const std::string& name);
  void reload();

 private:
  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

  SlangSession* slangSession = nullptr;
  void compileShader(Instance, SlangStage);
};
