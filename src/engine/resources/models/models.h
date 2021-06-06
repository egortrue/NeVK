#pragma once

// Сторонние библиотеки
#include <tiny_obj_loader.h>
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "commands.h"
#include "resources.h"
#include "textures.h"

// Стандартные библиотеки
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <utility>
#include <unordered_map>

class Models {
 public:
  typedef Models* Manager;

  typedef struct vertex_t {
    glm::float3 position;
    glm::float2 uv;
  } * Vertex;

  typedef struct model_t {
    // Метаданные
    std::string name;
    std::string objPath;
    std::string mtlPath;

    struct shape_t {
      uint32_t verticesCount;
      uint32_t diffuseTextureID;

      VkBuffer vertexBuffer;
      VkDeviceMemory vertexBufferMemory;
    };

    std::vector<shape_t*> shapes;
  } * Instance;

 private:
  Commands::Manager commands;
  Resources::Manager resources;
  Textures::Manager textures;

  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

 public:
  Models(Commands::Manager, Resources::Manager, Textures::Manager);
  ~Models();

  Instance load(const std::string& name);
  Instance get(const std::string& name);
  void destroy(const std::string& name);

 private:
  void parseData(Instance, tinyobj::ObjReader&);
};
