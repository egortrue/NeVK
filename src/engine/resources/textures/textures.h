#pragma once

// Сторонние библиотеки
#include <stb_image.h>

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"

// Стандартные библиотеки
#include <list>
#include <unordered_map>

typedef class Textures* TexturesManager;

class Textures {
 private:
  CoreManager core;
  CommandsManager commands;
  ResourcesManager resources;

  struct texture_t {
    int width, height;
    VkImage image;
    VkImageView view;
    VkDeviceSize size;
    VkDeviceMemory memory;
  };

  std::vector<texture_t*> handlers;
  std::unordered_map<std::string, uint32_t> idList;

 public:
  Textures(CoreManager, CommandsManager, ResourcesManager);
  ~Textures();

  VkSampler sampler;
  texture_t* loadTexture(const std::string& name);
  texture_t* getTexture(const std::string& name);
  void destroyTexture(const std::string& name);

 private:
  texture_t* createTexture(const std::string& name);
  void createTextureSampler();
  void destroyTextureSampler();
};
