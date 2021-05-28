#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "textures.h"

Textures::Textures(Core::Manager core, Commands::Manager commands, Resources::Manager resources) {
  this->core = core;
  this->commands = commands;
  this->resources = resources;
  createTextureSampler();
}

Textures::~Textures() {
  destroyTextureSampler();
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
};

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

void Textures::createTextureSampler() {
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = core->physicalDeviceProperties.limits.maxSamplerAnisotropy;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  if (vkCreateSampler(core->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create texture sampler!");
}

void Textures::destroyTextureSampler() {
  if (sampler != nullptr)
    vkDestroySampler(core->device, sampler, nullptr);
}
