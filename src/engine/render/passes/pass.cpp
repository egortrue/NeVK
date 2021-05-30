#include "pass.h"

void Pass::init() {
  createDescriptorSetsLayout();
  createDescriptorSets();
  updateDescriptorSets();
  createShaderModules();
  createRenderPass();
  createPipeline();
}

void Pass::destroy() {
  vkDestroyPipeline(core->device, pipeline.instance, nullptr);
  vkDestroyPipelineLayout(core->device, pipeline.layout, nullptr);
  vkDestroyRenderPass(core->device, pipeline.pass, nullptr);

  for (auto desciptorSetLayout : descriptorSetsLayout)
    vkDestroyDescriptorSetLayout(core->device, desciptorSetLayout, nullptr);
}

void Pass::reload() {
  vkDestroyPipeline(core->device, pipeline.instance, nullptr);
  vkDestroyPipelineLayout(core->device, pipeline.layout, nullptr);
  vkDestroyRenderPass(core->device, pipeline.pass, nullptr);

  updateDescriptorSets();
  createShaderModules();
  createRenderPass();
  createPipeline();
}

void Pass::resize() {
  vkDestroyPipeline(core->device, pipeline.instance, nullptr);
  vkDestroyPipelineLayout(core->device, pipeline.layout, nullptr);
  vkDestroyRenderPass(core->device, pipeline.pass, nullptr);

  createRenderPass();
  createPipeline();
}
