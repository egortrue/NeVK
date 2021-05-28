#include "shaders.h"

Shaders::Shaders() {
  slangSession = spCreateSession(NULL);
}

Shaders::~Shaders() {
  if (slangSession != nullptr)
    spDestroySession(slangSession);
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

  int entryPointIndex;
  if (stage == SLANG_STAGE_VERTEX)
    entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, shader->vertexName.c_str(), SLANG_STAGE_VERTEX);
  else if (stage == SLANG_STAGE_FRAGMENT)
    entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, shader->fragmentName.c_str(), SLANG_STAGE_FRAGMENT);

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
  std::vector<char>* shaderData;
  if (stage == SLANG_STAGE_VERTEX)
    shaderData = &shader->vertexCode;
  else if (stage == SLANG_STAGE_FRAGMENT)
    shaderData = &shader->fragmentCode;
  shaderData->clear();
  shaderData->resize(dataSize);
  memcpy(shaderData->data(), data, dataSize);

  spDestroyCompileRequest(slangRequest);
}

Shaders::Instance Shaders::loadShader(const std::string& name, const std::string& vertexName, const std::string& fragmentName) {
  // Найдем уже загруженный шейдер
  auto el = idList.find(name);
  if (el != idList.end()) {
    // Перезагрузка шейдера
    compileShader(handlers[el->second], SLANG_STAGE_VERTEX);
    compileShader(handlers[el->second], SLANG_STAGE_FRAGMENT);
    return handlers[el->second];
  }

  // Сохраним новые данные
  Instance shader = new shader_t;
  shader->name = name;
  shader->vertexName = vertexName;
  shader->fragmentName = fragmentName;
  compileShader(shader, SLANG_STAGE_VERTEX);
  compileShader(shader, SLANG_STAGE_FRAGMENT);
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name, id));
  handlers.push_back(shader);

  return shader;
}

void Shaders::reloadShader(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error("ERROR: shader was not loaded: " + name);

  compileShader(handlers[el->second], SLANG_STAGE_VERTEX);
  compileShader(handlers[el->second], SLANG_STAGE_FRAGMENT);
}

void Shaders::reload() {
  for (auto shader : handlers) {
    compileShader(shader, SLANG_STAGE_VERTEX);
    compileShader(shader, SLANG_STAGE_FRAGMENT);
  }
}
