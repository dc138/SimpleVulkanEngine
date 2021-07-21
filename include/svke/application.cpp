#include "application.hpp"

#include "defines.hpp"
#include "pch.hpp"

void sierpinski(std::vector<svke::Model::Vertex> &vertices, uint8_t depth,
                svke::Model::Vertex top, svke::Model::Vertex left,
                svke::Model::Vertex right) {
  if (depth <= 0) {
    vertices.push_back(top);
    vertices.push_back(right);
    vertices.push_back(left);

  } else {
    svke::Model::Vertex leftTop = {0.5f * (left.position + top.position),
                                   0.5f * (left.color + top.color)};
    svke::Model::Vertex rightTop = {0.5f * (right.position + top.position),
                                    0.5f * (right.color + top.color)};
    svke::Model::Vertex leftRight = {0.5f * (left.position + right.position),
                                     0.5f * (left.color + right.color)};

    sierpinski(vertices, depth - 1, left, leftRight, leftTop);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top);
  }
}

namespace svke {
Application::Application(uint32_t width, uint32_t height,
                         const std::string &window_name)
    : pWidth{width}, pHeight{height}, pWindowName{window_name} {
  pLoadGameObjects();
  pCreatePipelineLayout();
  pRecreateSwapChain();
  pCreateCommandBuffers();
}

Application::~Application() {
  vkDestroyPipelineLayout(pDevice.getDevice(), pPipelineLayout, nullptr);
}

void Application::Run() {
  while (!pWindow.ShouldClose()) {
    glfwPollEvents();
    pDrawFrame();
  }

  vkDeviceWaitIdle(pDevice.getDevice());
}

void Application::pLoadGameObjects() {
  std::vector<Model::Vertex> vertices{};

  sierpinski(vertices, 3, {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
             {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
             {{0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}});

  auto model = std::make_shared<Model>(pDevice, vertices);

  auto triangle = GameObject::CreateGameObject();
  triangle.uModel = model;
  triangle.uTransform2d.translation.x = .2f;
  triangle.uTransform2d.translation.y = -.3f;
  triangle.uTransform2d.scale = {.5f, 1.5f};
  triangle.uTransform2d.rotation = .25f * glm::two_pi<float>();

  pGameObjects.push_back(std::move(triangle));
}

void Application::pCreatePipelineLayout() {
  VkPushConstantRange push_constant_range{};

  push_constant_range.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constant_range.offset = 0;
  push_constant_range.size = sizeof(TestPushConstantData);

  VkPipelineLayoutCreateInfo pipeline_layout_info{};

  pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipeline_layout_info.setLayoutCount = 0;
  pipeline_layout_info.pSetLayouts = nullptr;
  pipeline_layout_info.pushConstantRangeCount = 1;
  pipeline_layout_info.pPushConstantRanges = &push_constant_range;

  if (vkCreatePipelineLayout(pDevice.getDevice(), &pipeline_layout_info,
                             nullptr, &pPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }
}

void Application::pCreatePipeline() {
  if (pSwapChain == nullptr) {
    throw std::runtime_error("Cannot create pipeline befor swap chain");

  } else if (pPipelineLayout == nullptr) {
    throw std::runtime_error("Cannot create pipeline before pipeline layout");
  }

  PipelineConfig pipeline_config{};
  Pipeline::DefaultPipelineConfig(pipeline_config);

  pipeline_config.render_pass = pSwapChain->getRenderPass();
  pipeline_config.pipeline_layout = pPipelineLayout;

  pPipeline =
      std::make_unique<Pipeline>(pDevice, "shaders/simple.vert.spv",
                                 "shaders/simple.frag.spv", pipeline_config);
}

void Application::pCreateCommandBuffers() {
  pCommandBuffer.resize(pSwapChain->getImageCount());

  VkCommandBufferAllocateInfo alloc_info{};

  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandPool = pDevice.getCommandPool();
  alloc_info.commandBufferCount = static_cast<uint32_t>(pCommandBuffer.size());

  if (vkAllocateCommandBuffers(pDevice.getDevice(), &alloc_info,
                               pCommandBuffer.data()) != VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate command buffers");
  }
}

void Application::pFreeCommandBuffers() {
  vkFreeCommandBuffers(pDevice.getDevice(), pDevice.getCommandPool(),
                       static_cast<uint32_t>(pCommandBuffer.size()),
                       pCommandBuffer.data());
  pCommandBuffer.clear();
}

void Application::pDrawFrame() {
  uint32_t image_index;
  VkResult result = pSwapChain->AcquireNextImage(&image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    pRecreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("Failed to acquire swap chain image");
  }

  pRecordCommandBuffer(image_index);
  result = pSwapChain->SubmitCommandBuffers(&pCommandBuffer[image_index],
                                            &image_index);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      pWindow.WasResized()) {
    pWindow.ResetResize();
    pRecreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS) {
    throw std::runtime_error("Failed to present swap chain image");
  }
}

void Application::pRecreateSwapChain() {
  auto extent = pWindow.getExtent();

  while (extent.width == 0 || extent.height == 0) {
    extent = pWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(pDevice.getDevice());

  if (pSwapChain == nullptr) {
    pSwapChain = std::make_unique<SwapChain>(pDevice, extent);
  } else {
    pSwapChain =
        std::make_unique<SwapChain>(pDevice, extent, std::move(pSwapChain));

    if (pSwapChain->getImageCount() != pCommandBuffer.size()) {
      pFreeCommandBuffers();
      pCreateCommandBuffers();
    }
  }

  pCreatePipeline();
}

void Application::pRecordCommandBuffer(uint32_t image_index) {
  VkCommandBufferBeginInfo begin_info{};

  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(pCommandBuffer[image_index], &begin_info) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to begin recording command buffer");
  }

  VkRenderPassBeginInfo render_pass_info{};

  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = pSwapChain->getRenderPass();
  render_pass_info.framebuffer = pSwapChain->getFrameBuffer(image_index);

  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = pSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clear_values{};

  clear_values[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
  clear_values[1].depthStencil = {1.0f, 0};
  render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
  render_pass_info.pClearValues = clear_values.data();

  vkCmdBeginRenderPass(pCommandBuffer[image_index], &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{};

  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(pSwapChain->getSwapChainExtent().width);
  viewport.height = static_cast<float>(pSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor{};

  scissor.offset = {0, 0};
  scissor.extent = pSwapChain->getSwapChainExtent();

  vkCmdSetViewport(pCommandBuffer[image_index], 0, 1, &viewport);
  vkCmdSetScissor(pCommandBuffer[image_index], 0, 1, &scissor);

  pRenderGameObjects(pCommandBuffer[image_index]);

  vkCmdEndRenderPass(pCommandBuffer[image_index]);

  if (vkEndCommandBuffer(pCommandBuffer[image_index]) != VK_SUCCESS) {
    throw std::runtime_error("Failed to record command buffer");
  }
}

void Application::pRenderGameObjects(VkCommandBuffer command_buffer) {
  pPipeline->Bind(command_buffer);

  for (auto &object : pGameObjects) {
    TestPushConstantData push{};

    push.offset = object.uTransform2d.translation;
    push.transform = object.uTransform2d.getMat2();

    vkCmdPushConstants(command_buffer, pPipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(TestPushConstantData), &push);

    object.uModel->Bind(command_buffer);
    object.uModel->Draw(command_buffer);
  }
}
} // namespace svke
