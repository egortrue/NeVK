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

class Textures {
 public:
  typedef Textures* Manager;
  typedef struct texture_t {
    int width, height;
    VkImage image;
    VkImageView view;
    VkSampler sampler;
    VkDeviceSize size;
    VkDeviceMemory memory;
  } * Instance;

 private:
  Core::Manager core;
  Commands::Manager commands;
  Resources::Manager resources;

  std::vector<Instance> handlers;
  std::unordered_map<std::string, uint32_t> idList;

 public:
  Textures(Core::Manager, Commands::Manager, Resources::Manager);
  ~Textures();

  VkSampler commonSampler;
  Instance loadTexture(const std::string& name);
  Instance getTexture(const std::string& name);
  void destroyTexture(const std::string& name);

 private:
  texture_t* createTexture(const std::string& name);
  void createTextureSampler();
  void destroyTextureSampler();
};
