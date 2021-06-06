#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "textures.h"

Textures::Textures(Core::Manager core) {
  this->core = core;
}

Textures::~Textures() {
  for (auto texture : handlers) {
    core->resources->destroyImageView(texture->view);
    core->resources->destroyImage(texture->image, texture->memory);
    delete texture;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Textures::Instance Textures::load(const std::string& name) {
  // Найдем уже загруженную текстуру
  auto el = idList.find(name);
  if (el != idList.end())
    return handlers[el->second];

  // Запишем новую текстуру
  texture_t* texture = create(name);
  uint32_t id = static_cast<uint32_t>(handlers.size());
  idList.insert(std::make_pair(name, id));
  handlers.push_back(texture);

  std::cout << "Texture \"" << name << "\" was loaded successfully" << std::endl;
  return handlers[id];
}

Textures::Instance Textures::get(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to get texture: ") + name);
  return handlers[el->second];
}

void Textures::getViews(std::vector<VkImageView>& views) {
  views.clear();
  for (auto texture : handlers)
    views.push_back(texture->view);
}

uint32_t Textures::getID(const std::string& name) {
  auto el = idList.find(name);
  if (el != idList.end())
    return el->second;
  throw std::runtime_error(std::string("ERROR: Failed to get texture: ") + name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

Textures::Instance Textures::create(const std::string& name) {
  Instance texture = new texture_t;

  // Получим изображение в виде набора пикселов
  stbi_uc* pixels = stbi_load(name.c_str(), &texture->width, &texture->height, nullptr, STBI_rgb_alpha);
  texture->size = texture->width * texture->height * 4;
  if (!pixels)
    throw std::runtime_error(std::string("ERROR: Failed to load texture image: ") + name);

  // Создание изображения для хранения текстуры
  core->resources->createImage(
      texture->width, texture->height,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_TILING_OPTIMAL,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      texture->image, texture->memory);

  // Заполнение изображения данными
  core->commands->copyDataToImage(pixels, texture->image, texture->size, texture->width, texture->height);

  // Удалим сырые данные
  stbi_image_free(pixels);

  // Создание вида изображения
  texture->view = core->resources->createImageView(
      texture->image,
      VK_FORMAT_R8G8B8A8_SRGB,
      VK_IMAGE_ASPECT_COLOR_BIT);

  return texture;
}

void Textures::destroy(const std::string& name) {
  auto el = idList.find(name);
  if (el == idList.end())
    throw std::runtime_error(std::string("ERROR: Failed to destroy texture: ") + name);
  auto texture = handlers[el->second];
  core->resources->destroyImageView(texture->view);
  core->resources->destroyImage(texture->image, texture->memory);
  handlers.erase(handlers.begin() + el->second);
  idList.erase(el);
  delete texture;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
