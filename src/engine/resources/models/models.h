#pragma once

// Сторонние библиотеки
#include <tiny_obj_loader.h>
#include <glm/gtx/compatibility.hpp>

// Стандартные библиотеки
#include <iostream>
#include <array>

typedef class Models* ModelsManager;

class Models {
 private:
  struct vertex_t {
    glm::float3 position;
    glm::float2 uv;
  };

  struct model_t {
    std::vector<float> vertices;
    std::vector<uint32_t> indices;
  };

 public:
  model_t cube;
  Models();
  ~Models();
};
