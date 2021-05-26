#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "models.h"

Models::Models(CommandsManager commands, ResourcesManager resources) {
  this->commands = commands;
  this->resources = resources;
}

Models::~Models() {
  for (auto model : handlers) {
    destroyModelBuffers(model);
    delete model;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Models::Instance Models::loadModel(const std::string& name) {
  // Найдем уже загруженную текстуру
  auto el = idList.find(name);
  if (el != idList.end())
    return handlers[el->second];

  // Инициализация модели
  Instance model = new model_t;
  model->name = name;
  model->poligonsCount = 0;
  model->verticesCount = 0;

  // Чтение данных из файла .obj
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = "misc/models/";
  reader.ParseFromFile(name, readerConfig);

  // Проверка корректного чтения
  if (!reader.Error().empty())
    throw std::runtime_error("ERROR: " + reader.Error());
  else if (!reader.Warning().empty())
    std::cerr << "WARNING [TinyObjReader]:" << reader.Warning();

  // Получение данных от TOL
  parseModelData(model, reader);

  // Создание буферов для конвейера
  createModelBuffers(model);

  // Запись модели
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name, id));
  handlers.push_back(model);

  std::cout << "Model \"" << name << "\" was loaded successfully" << std::endl;
  return handlers[id];
}

Models::Instance Models::getModel(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to get model: ") + name);
  return handlers[el->second];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Models::parseModelData(Instance model, tinyobj::ObjReader& reader) {
  // Данные TOL
  auto& attributes = reader.GetAttrib();  // Координаты, нормали, UV, ...
  auto& shapes = reader.GetShapes();      // Объекты, в виде наборов индексов атрибуты

  // Прочитаем каждый объект
  for (auto& shape : shapes) {
    // Прочитаем каждый полигон (face)
    size_t index_offset = 0;
    size_t faces_count = shape.mesh.num_face_vertices.size();
    model->poligonsCount += faces_count;

    for (size_t face = 0; face < faces_count; face++) {
      // Прочитаем каждую вершину в полигоне
      size_t face_vertices_count = shape.mesh.num_face_vertices[face];
      model->verticesCount += face_vertices_count;

      for (size_t vertex = 0; vertex < face_vertices_count; vertex++) {
        // Получим данные индексации
        auto idx = shape.mesh.indices[index_offset + vertex];

        // Координаты вершины
        model->vertices.push_back(attributes.vertices[3 * idx.vertex_index + 0]);
        model->vertices.push_back(attributes.vertices[3 * idx.vertex_index + 1]);
        model->vertices.push_back(attributes.vertices[3 * idx.vertex_index + 2]);

        // UV координаты вершины
        model->vertices.push_back(attributes.texcoords[2 * idx.texcoord_index + 0]);
        model->vertices.push_back(attributes.texcoords[2 * idx.texcoord_index + 1]);
      }
      index_offset += face_vertices_count;
    }
  }

  // TODO: Сделать индексацию
  // indicies - массив от 0 до количества всех вершин в модели
  // так как мы храним данные абсолютно всех вершин
  model->indices.resize(model->verticesCount);
  for (int i = 0; i < model->verticesCount; i++)
    model->indices[i] = i;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Models::createModelBuffers(Instance model) {
  resources->createBuffer(
      model->vertices.size() * sizeof(float),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      model->vertexBuffer, model->vertexBufferMemory);

  commands->copyDataToBuffer(
      model->vertices.data(),
      model->vertexBuffer,
      model->vertices.size() * sizeof(float));

  resources->createBuffer(
      model->indices.size() * sizeof(uint32_t),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      model->indexBuffer, model->indexBufferMemory);

  commands->copyDataToBuffer(
      model->indices.data(),
      model->indexBuffer,
      model->indices.size() * sizeof(uint32_t));
}

void Models::destroyModelBuffers(Instance model) {
  resources->destroyBuffer(model->vertexBuffer, model->vertexBufferMemory);
  resources->destroyBuffer(model->indexBuffer, model->indexBufferMemory);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
