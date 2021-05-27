#pragma once

// Сторонние библиотеки
#include <tiny_obj_loader.h>
#include <glm/gtx/compatibility.hpp>

// Внутренние библиотеки
#include "commands.h"
#include "resources.h"

// Стандартные библиотеки
#include <iostream>
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
    uint32_t verticesCount;
    uint32_t poligonsCount;

    // Вершины
    std::vector<float> vertices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    // Индексы соответвующих вершин
    std::vector<uint32_t> indices;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
  } * Instance;

 private:
  Commands::Manager commands;
  Resources::Manager resources;

  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

 public:
  Models(Commands::Manager, Resources::Manager);
  ~Models();

  Instance loadModel(const std::string& name);
  Instance getModel(const std::string& name);
  void destroyModel(const std::string& name);

 private:
  void parseModelData(Instance, tinyobj::ObjReader&);
  void createModelBuffers(Instance);
  void destroyModelBuffers(Instance);
};
