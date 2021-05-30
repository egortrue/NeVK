#include "shaders.h"

Shaders::Shaders(Core::Manager core) {
  this->core = core;
  slangSession = spCreateSession(NULL);
}

Shaders::~Shaders() {
  if (slangSession != nullptr)
    spDestroySession(slangSession);
  for (auto shader : handlers)
    destroyShader(shader);
}

void Shaders::compileShader(Instance shader, SlangStage stage) {
  SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

  // Опции компиляции
  // spSetDebugInfoLevel(slangRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
  int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SPIRV);
  SlangProfileID profileID = spFindProfile(slangSession, "sm_6_3");
  spSetTargetProfile(slangRequest, targetIndex, profileID);
  int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
  spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, shader->name.c_str());
  int entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, shader->entryPoint.c_str(), stage);

  // Компиляция шейдера в SPIR-V
  const SlangResult compileRes = spCompile(slangRequest);
  auto diagnostics = spGetDiagnosticOutput(slangRequest);
  if (SLANG_FAILED(compileRes)) {
    spDestroyCompileRequest(slangRequest);
    throw std::runtime_error(diagnostics);
  }

  // Получение кода SPIR-V
  size_t dataSize = 0;
  void const* data = spGetEntryPointCode(slangRequest, entryPointIndex, &dataSize);
  if (!data)
    throw std::runtime_error(diagnostics);

  // Запись данных в дискриптор
  shader->code.clear();
  shader->code.resize(dataSize);
  memcpy(shader->code.data(), data, dataSize);

  spDestroyCompileRequest(slangRequest);

  // Создание шейдерного модуля Vulkan
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.codeSize = dataSize;
  createInfo.pCode = reinterpret_cast<const uint32_t*>(data);
  if (vkCreateShaderModule(core->device, &createInfo, nullptr, &shader->module) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create shader module!");
}

Shaders::Instance Shaders::loadShader(const std::string& name, const std::string& entryPoint, SlangStage stage) {
  // Найдем уже загруженный шейдер
  auto el = idList.find(name + entryPoint);
  if (el != idList.end()) {
    // Перезагрузка шейдера
    compileShader(handlers[el->second], handlers[el->second]->stage);
    return handlers[el->second];
  }

  // Сохраним новые данные
  Instance shader = new shader_t;
  shader->name = name;
  shader->entryPoint = entryPoint;
  compileShader(shader, stage);
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name + entryPoint, id));
  handlers.push_back(shader);

  return shader;
}

void Shaders::reloadShader(const std::string& name, const std::string& entryPoint) {
  auto el = idList.find(name + entryPoint);
  if (el == idList.end())
    throw std::runtime_error("ERROR: shader was not loaded: " + name);
  compileShader(handlers[el->second], handlers[el->second]->stage);
}

void Shaders::reload() {
  for (auto shader : handlers)
    compileShader(shader, shader->stage);
}

void Shaders::destroyShader(Instance shader) {
  vkDestroyShaderModule(core->device, shader->module, nullptr);
  delete shader;
}
