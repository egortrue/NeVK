#pragma once

// Сторонние библиотеки
#include "tiny_obj_loader.h"

// Стандартные библиотеки
#include <iostream>
#include <array>

typedef class Models* ModelsManager;

class Models {
 private:
  struct vertex_t {
    std::array<float, 3> position;
    std::array<float, 2> uv;
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
