#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "textures.h"

Textures::Textures(Core::Manager core, Commands::Manager commands, Resources::Manager resources) {
  this->core = core;
  this->commands = commands;
  this->resources = resources;
}

Textures::~Textures() {
  for (auto texture : handlers) {
    resources->destroyImageView(texture->view);
    resources->destroyImage(texture->image, texture->memory);
    delete texture;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Textures::Instance Textures::loadTexture(const std::string& name) {
  // Найдем уже загруженную текстуру
  auto el = idList.find(name);
  if (el != idList.end())
    return handlers[el->second];

  // Запишем новую текстуру
  texture_t* texture = createTexture(name);
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name, id));
  handlers.push_back(texture);

  std::cout << "Texture \"" << name << "\" was loaded successfully" << std::endl;
  return handlers[id];
}

Textures::Instance Textures::getTexture(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to get texture: ") + name);
  return handlers[el->second];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Textures::Instance Textures::createTexture(const std::string& name) {
  Instance texture = new texture_t;

  // Получим изображение в виде набора пикселов
  stbi_uc* pixels = stbi_load(name.c_str(), &texture->width, &texture->height, nullptr, STBI_rgb_alpha);
  texture->size = texture->width * texture->height * 4;
  if (!pixels)
    throw std::runtime_error(std::string("ERROR: Failed to load texture image: ") + name);

  // Создание изображения для хранения текстуры
  resources->createImage(
      texture->width, texture->height,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      texture->image, texture->memory);

  // Заполнение изображения данными
  commands->copyDataToImage(pixels, texture->image, texture->size, texture->width, texture->height);

  // Удалим сырые данные
  stbi_image_free(pixels);

  // Создание вида изображения
  texture->view = resources->createImageView(
      texture->image,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_ASPECT_COLOR_BIT);

  return texture;
}

void Textures::destroyTexture(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to destroy texture: ") + name);
  auto texture = handlers[el->second];
  resources->destroyImageView(texture->view);
  resources->destroyImage(texture->image, texture->memory);
  handlers.erase(handlers.begin() + el->second);
  idList.erase(el);
  delete texture;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
