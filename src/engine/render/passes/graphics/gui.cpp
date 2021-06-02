#include "gui.h"

void GUI::update(uint32_t index) {
  updateUI();
}

void GUI::reload() {
  for (auto framebuffer : framebuffers)
    vkDestroyFramebuffer(core->device, framebuffer, nullptr);
  vkDestroyRenderPass(core->device, pipeline.pass, nullptr);

  createRenderPass();
  createFramebuffers();
}

void GUI::resize() {
  reload();
}

void GUI::destroy() {
  GraphicsPass::destroy();
  ImGui_ImplVulkan_Shutdown();
  ImGui::DestroyContext();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUI::init() {
  createRenderPass();
  createFramebuffers();

  //==================================
  // ImGUI

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  imgui.io = ImGui::GetIO();
  imgui.io.DisplaySize = ImVec2(window->width, window->height);
  imgui.io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

  // Основые параметры инициализации
  imgui.init.Instance = core->instance;
  imgui.init.Device = core->device;
  imgui.init.PhysicalDevice = core->physicalDevice.handler;
  imgui.init.Queue = core->graphicsQueue;
  imgui.init.QueueFamily = core->queueFamily.graphicsFamily.value();
  imgui.init.ImageCount = core->swapchain.count;
  imgui.init.MinImageCount = 2;
  imgui.init.DescriptorPool = resources->descriptorPool;

  ImGui_ImplGlfw_InitForVulkan(window->instance, true);
  ImGui_ImplVulkan_Init(&imgui.init, pipeline.pass);

  // Загрузка стандартных шрифтов
  VkCommandBuffer cmd = commands->beginSingleTimeCommands();
  ImGui_ImplVulkan_CreateFontsTexture(cmd);
  commands->endSingleTimeCommands(cmd);
  ImGui_ImplVulkan_DestroyFontUploadObjects();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUI::updateUI() {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Menu", nullptr, WINDOW_NOMOVE | WINDOW_AUTOSIZE);
  {
    ImGui::SetWindowPos(ImVec2(window->width - ImGui::GetWindowWidth() - 10, 10));
    imgui.menuHovered = ImGui::IsWindowHovered();

    ImGui::Text("Window");
    ImGui::Text("   Title");
    ImGui::SameLine();
    ImGui::InputText("###title", window->title.data(), 255);
    ImGui::SameLine();
    if (ImGui::Button("Change"))
      window->setTitle(window->title.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("    Size", window->width, window->height);
    ImGui::SameLine();
    float window_size[] = {static_cast<float>(window->width),
                           static_cast<float>(window->height)};
    ImGui::InputFloat2("###window_size", window_size);

    ImGui::Separator();
    //================================================

    ImGui::Text("Camera");
    auto camera = scene->getCamera();
    float position[] = {camera->transform.position.x,
                        camera->transform.position.y,
                        camera->transform.position.z};
    ImGui::Text("Position");
    ImGui::SameLine();
    ImGui::InputFloat3("###camera_position", position);
    camera->transform.position = {position[0], position[1], position[2]};

    float rotation[] = {camera->transform.rotation.x,
                        camera->transform.rotation.y,
                        camera->transform.rotation.z};
    ImGui::Text("Rotation");
    ImGui::SameLine();
    ImGui::InputFloat3("###camera_rotation", rotation);
    camera->transform.rotation = {rotation[0], rotation[1], rotation[2]};

    float scale[] = {camera->transform.scale.x,
                     camera->transform.scale.y,
                     camera->transform.scale.z};
    ImGui::Text("   Scale");
    ImGui::SameLine();
    ImGui::InputFloat3("###camera_scale", scale);
    camera->transform.scale = {scale[0], scale[1], scale[2]};

    ImGui::Separator();
    //================================================

    ImGui::Text("Object");
    std::vector<const char*> names;
    names.clear();
    for (auto object : scene->objects)
      names.push_back(object->model->name.c_str());

    ImGui::Text("Instance");
    ImGui::SameLine();
    ImGui::Combo(reinterpret_cast<const char*>("###object_list"),
                 reinterpret_cast<int*>(&scene->currentObject),
                 names.data(), static_cast<int>(names.size()));
    auto object = scene->objects[scene->currentObject];

    float object_position[] = {object->transform.position.x,
                               object->transform.position.y,
                               object->transform.position.z};
    ImGui::Text("Position");
    ImGui::SameLine();
    ImGui::InputFloat3("###object_position", object_position);
    object->transform.position = {object_position[0], object_position[1], object_position[2]};

    float object_rotation[] = {object->transform.rotation.x,
                               object->transform.rotation.y,
                               object->transform.rotation.z};
    ImGui::Text("Rotation");
    ImGui::SameLine();
    ImGui::InputFloat3("###object_rotation", object_rotation);
    object->transform.rotation = {object_rotation[0], object_rotation[1], object_rotation[2]};

    float object_scale[] = {object->transform.scale.x,
                            object->transform.scale.y,
                            object->transform.scale.z};
    ImGui::Text("   Scale");
    ImGui::SameLine();
    ImGui::InputFloat3("###object_scale", object_scale);
    object->transform.scale = {object_scale[0], object_scale[1], object_scale[2]};

    ImGui::Separator();

    ImGui::End();
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUI::record(uint32_t index, VkCommandBuffer cmd) {
  ImGui::Render();
  ImDrawData* draw_data = ImGui::GetDrawData();

  const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
  if (!is_minimized) {
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    memcpy(&imgui.data.ClearValue.color.float32[0], &clear_color, 4 * sizeof(float));
  }

  VkRenderPassBeginInfo info = {};
  info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  info.renderPass = pipeline.pass;
  info.framebuffer = framebuffers[index];
  info.renderArea.extent.width = target.width;
  info.renderArea.extent.height = target.height;
  info.clearValueCount = 1;
  info.pClearValues = &imgui.data.ClearValue;
  vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);

  ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);

  vkCmdEndRenderPass(cmd);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUI::createFramebuffers() {
  framebuffers.resize(target.views.size());
  for (size_t i = 0; i < target.views.size(); ++i) {
    std::vector<VkImageView> attachment = {target.views[i]};
    framebuffers[i] = createFramebuffer(attachment, target.width, target.height);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUI::createRenderPass() {
  //=================================================================================
  // Описание цветового подключения - выходного изображения конвейера

  VkAttachmentDescription colorAttachment{};
  colorAttachment.format = target.format;

  // Действия при работе с изображением
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  // Действия при работе с трафаретом
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  // Раскладка изображения
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;  // Не устанавливается автоматически
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;             // Устанавливается автоматически в конце прохода

  // Мультисэмплинг
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  // Число образцов (1 = выкл)

  // Элемент подпрохода
  VkAttachmentReference colorAttachmentRef{};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  //=================================================================================
  // Подпроходы рендера

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

  // Цветовые подключения
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  //=================================================================================
  // Зависимости подпроходов рендера

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  //=================================================================================
  // Создание прохода рендера

  std::vector<VkAttachmentDescription> attachments = {colorAttachment};

  VkRenderPassCreateInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(core->device, &renderPassInfo, nullptr, &pipeline.pass) != VK_SUCCESS)
    throw std::runtime_error("ERROR: Failed to create render pass!");
}
