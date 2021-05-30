#pragma once

// Внутренние библиотеки
#include "core.h"
#include "commands.h"
#include "resources.h"
#include "shaders.h"

// Стандартные библиотеки
#include <array>
#include <string>
#include <vector>

class Pass {
 protected:
  Core::Manager core;
  Commands::Manager commands;
  Resources::Manager resources;
  Shaders::Manager shaders;

  struct pipeline_t {
    VkPipeline instance;
    VkPipelineLayout layout;
    VkPipelineCache cache;
    VkRenderPass pass;
  } pipeline;

  virtual void init();
  virtual void destroy();
  virtual void reload();  // Полная перезагрузка
  virtual void resize();  // Частичная перезагрузка

  virtual void createPipeline() = 0;
  virtual void createRenderPass() = 0;

  // Шейдеры
  std::string shaderName;
  virtual void createShaderModules() = 0;

  // Подключаемые ресурсы
  std::vector<VkDescriptorSet> descriptorSets;
  std::vector<VkDescriptorSetLayout> descriptorSetsLayout;
  virtual void createDescriptorSetsLayout() = 0;  // Описание используемых ресурсов
  virtual void createDescriptorSets() = 0;        // Выделение памяти под ресурсы
  virtual void updateDescriptorSets() = 0;        // Подключение данных к выделенной памяти
};
