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

typedef class Models* ModelsManager;

class Models {
 private:
  CommandsManager commands;
  ResourcesManager resources;

 public:
  struct vertex_t {
    glm::float3 position;
    glm::float2 uv;
  };

  typedef struct model_t {
    // Метаданные
    std::string name;
    uint32_t verticesCount;  // Количество вершин
    uint32_t poligonsCount;  // Количество полигонов

    // Вершины
    std::vector<float> vertices;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    // Индексы соответвующих вершин
    std::vector<uint32_t> indices;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
  } * Instance;

  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

  Models(CommandsManager, ResourcesManager);
  ~Models();

  Instance loadModel(const std::string& name);
  Instance getModel(const std::string& name);
  void destroyModel(const std::string& name);

 private:
  void parseModelData(Instance, tinyobj::ObjReader&);
  void createModelBuffers(Instance);
  void destroyModelBuffers(Instance);
};
