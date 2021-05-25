#include "shaders.h"

Shaders::Shaders() {
  slangSession = spCreateSession(NULL);
}

Shaders::~Shaders() {
  if (slangSession != nullptr)
    spDestroySession(slangSession);
}

Shaders::shader_t Shaders::compileShader(const char* name, const char* entryPointName, SlangStage stage) {
  SlangCompileRequest* slangRequest = spCreateCompileRequest(slangSession);

  // spSetDebugInfoLevel(slangRequest, SLANG_DEBUG_INFO_LEVEL_MAXIMAL);
  int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_SPIRV);
  SlangProfileID profileID = spFindProfile(slangSession, "sm_6_3");
  spSetTargetProfile(slangRequest, targetIndex, profileID);
  int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
  spAddTranslationUnitSourceFile(slangRequest, translationUnitIndex, name);
  int entryPointIndex = spAddEntryPoint(slangRequest, translationUnitIndex, entryPointName, stage);

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

  // Создание нового дискриптора для шейдера
  shader_t shader{};
  shader.name = std::string(name);
  shader.entryPointName = std::string(entryPointName);
  shader.code.resize(dataSize);
  memcpy(&shader.code[0], data, dataSize);
  shader.slangReflection = (slang::ShaderReflection*)spGetReflection(slangRequest);
  shader.slangRequest = slangRequest;

  spDestroyCompileRequest(slangRequest);
  return shader;
}

uint32_t Shaders::loadShader(const char* name, const char* entryPointName, SlangStage stage) {
  shader_t new_shader = compileShader(name, entryPointName, stage);

  // Поиск и перезапись старой информации о шейдере
  uint32_t shaderId = 0;
  for (; shaderId < handlers.size(); ++shaderId) {
    shader_t& old_shader = handlers[shaderId];
    if (old_shader.name == new_shader.name && old_shader.entryPointName == new_shader.entryPointName) {
      old_shader.code.resize(new_shader.code.size());
      memcpy(&old_shader.code[0], &new_shader.code[0], new_shader.code.size());
      return shaderId;
    }
  }

  // Сохраним новые данные
  shaderId = handlers.size();
  handlers.push_back(new_shader);
  return shaderId;
}

void Shaders::reloadAllShaders() {
  for (const auto& shader : handlers)
    loadShader(shader.name.c_str(), shader.entryPointName.c_str(), shader.stage);
}

bool Shaders::getShaderCode(uint32_t id, const char*& code, uint32_t& size) {
  if (handlers.size() <= id) return false;
  shader_t& shader = handlers[id];
  code = shader.code.data();
  size = shader.code.size();
  return true;
}
