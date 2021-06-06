#define TINYOBJLOADER_IMPLEMENTATION
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "models.h"

Models::Models(Commands::Manager commands, Resources::Manager resources, Textures::Manager textures) {
  this->commands = commands;
  this->resources = resources;
  this->textures = textures;
}

Models::~Models() {
  for (auto model : handlers) {
    for (auto shape : model->shapes)
      resources->destroyBuffer(shape->vertexBuffer, shape->vertexBufferMemory);
    delete model;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Models::Instance Models::load(const std::string& name) {
  // Найдем уже загруженную модель
  auto el = idList.find(name);
  if (el != idList.end())
    return handlers[el->second];

  // Инициализация модели
  Instance model = new model_t;
  model->name = name;
  model->objPath = "misc\\models\\" + name + "\\" + name + ".obj";
  model->mtlPath = "misc\\models\\" + name;

  // Чтение данных из файла .obj
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = model->mtlPath;
  reader.ParseFromFile(model->objPath, readerConfig);

  // Проверка корректного чтения
  if (!reader.Error().empty())
    throw std::runtime_error("ERROR: " + reader.Error());
  else if (!reader.Warning().empty())
    std::cerr << "WARNING [TinyObjReader]:" << reader.Warning() << std::endl;

  // Получение данных от TOL
  parseData(model, reader);

  // Запись модели
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name, id));
  handlers.push_back(model);

  std::cout << "Model \"" << name << "\" was loaded successfully" << std::endl;
  return handlers[id];
}

Models::Instance Models::get(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to get model: ") + name);
  return handlers[el->second];
}

void Models::destroy(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end()) {
    std::cerr << "WARNING: Model was already destroyed: " << name << std::endl;
    return;
  }

  Instance model = handlers[el->second];
  for (auto shape : model->shapes)
    resources->destroyBuffer(shape->vertexBuffer, shape->vertexBufferMemory);
  handlers.erase(handlers.begin() + el->second);
  delete model;
  idList.erase(el);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void Models::parseData(Instance model, tinyobj::ObjReader& reader) {
  // Данные TOL
  auto& attributes = reader.GetAttrib();    // Координаты, нормали, UV, ...
  auto& shapes = reader.GetShapes();        // Объекты, в виде наборов индексов атрибуты
  auto& materials = reader.GetMaterials();  // Дополнительные параметры

  // Прочитаем каждый объект
  for (auto& shape : shapes) {
    size_t index_offset = 0;

    std::vector<float> bufferData;
    model_t::shape_t* shapeData = new model_t::shape_t;
    shapeData->verticesCount = 0;

    // Количество полигонов в объекте
    size_t faces_count = shape.mesh.num_face_vertices.size();

    // Прочитаем каждый полигон (face)
    for (size_t face = 0; face < faces_count; face++) {
      // Количество вершин в полигоне
      size_t face_vertices_count = shape.mesh.num_face_vertices[face];

      // Полигоны должны быть триангулированы
      assert(face_vertices_count == 3);

      // Прочитаем каждую вершину в полигоне
      for (size_t vertex = 0; vertex < face_vertices_count; vertex++) {
        // Индексированные данные
        auto idx = shape.mesh.indices[index_offset + vertex];
        uint32_t vertexIndex = idx.vertex_index;
        uint32_t texcoordIndex = idx.texcoord_index;

        // Мировые координаты
        bufferData.push_back(attributes.vertices[3 * vertexIndex + 0]);
        bufferData.push_back(attributes.vertices[3 * vertexIndex + 1]);
        bufferData.push_back(attributes.vertices[3 * vertexIndex + 2]);

        // Текстурные координаты
        bufferData.push_back(attributes.texcoords[2 * texcoordIndex + 0]);
        bufferData.push_back(1.0f - attributes.texcoords[2 * texcoordIndex + 1]);
      }

      // Переход к следующему полигону
      index_offset += face_vertices_count;
    }
    shapeData->verticesCount = bufferData.size() / 5;

    // Получим материалы объекта
    if (materials.empty()) continue;
    auto idx = shape.mesh.material_ids[0];

    // Загрузка текстур
    shapeData->diffuseTextureID = 0;
    if (materials[idx].diffuse_texname.length() > 1) {
      std::string texturePath = model->mtlPath + "\\" + materials[idx].diffuse_texname;
      textures->load(texturePath);
      shapeData->diffuseTextureID = textures->getID(texturePath);
    }

    // Отправка данных на устройство
    VkDeviceSize size = bufferData.size() * sizeof(float);
    resources->createBuffer(
        size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        shapeData->vertexBuffer, shapeData->vertexBufferMemory);

    commands->copyDataToBuffer(
        bufferData.data(),
        shapeData->vertexBuffer,
        size);

    model->shapes.push_back(shapeData);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
