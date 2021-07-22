#include "application.hpp"
#include "defines.hpp"
#include "pch.hpp"

void sierpinski(std::vector<svke::Model::Vertex> &vertices,
                uint8_t                           depth,
                svke::Model::Vertex               top,
                svke::Model::Vertex               left,
                svke::Model::Vertex               right) {
  if (depth <= 0) {
    vertices.push_back(top);
    vertices.push_back(right);
    vertices.push_back(left);

  } else {
    svke::Model::Vertex leftTop   = {0.5f * (left.position + top.position), 0.5f * (left.color + top.color)};
    svke::Model::Vertex rightTop  = {0.5f * (right.position + top.position), 0.5f * (right.color + top.color)};
    svke::Model::Vertex leftRight = {0.5f * (left.position + right.position), 0.5f * (left.color + right.color)};

    sierpinski(vertices, depth - 1, left, leftRight, leftTop);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top);
  }
}

namespace svke {
  Application::Application(uint32_t width, uint32_t height, const std::string &window_name)
      : pWidth {width}, pHeight {height}, pWindowName {window_name} {
    pLoadGameObjects();
    pCreatePipelineLayout();
    pCreatePipeline();
  }

  Application::~Application() { vkDestroyPipelineLayout(pDevice.getDevice(), pPipelineLayout, nullptr); }

  void Application::Run() {
    while (!pWindow.ShouldClose()) {
      glfwPollEvents();

      if (auto command_buffer = pRenderer.uBeginFrame()) {
        pRenderer.uBeginSwapChainRenderPass(command_buffer);
        pRenderGameObjects(command_buffer);
        pRenderer.uEndSwapChainRenderPass(command_buffer);
        pRenderer.uEndFrame();
      }
    }

    vkDeviceWaitIdle(pDevice.getDevice());
  }

  void Application::pLoadGameObjects() {
    std::vector<Model::Vertex> vertices {};

    sierpinski(vertices,
               3,
               {{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
               {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
               {{0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}});

    auto model = std::make_shared<Model>(pDevice, vertices);

    auto triangle                       = GameObject::CreateGameObject();
    triangle.uModel                     = model;
    triangle.uTransform2d.translation.x = .2f;
    triangle.uTransform2d.translation.y = -.3f;
    triangle.uTransform2d.scale         = {.5f, 1.5f};
    triangle.uTransform2d.rotation      = .25f * glm::two_pi<float>();

    pGameObjects.push_back(std::move(triangle));
  }

  void Application::pCreatePipelineLayout() {
    VkPushConstantRange push_constant_range {};

    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset     = 0;
    push_constant_range.size       = sizeof(TestPushConstantData);

    VkPipelineLayoutCreateInfo pipeline_layout_info {};

    pipeline_layout_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount         = 0;
    pipeline_layout_info.pSetLayouts            = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges    = &push_constant_range;

    if (vkCreatePipelineLayout(pDevice.getDevice(), &pipeline_layout_info, nullptr, &pPipelineLayout) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create pipeline layout");
    }
  }

  void Application::pCreatePipeline() {
    assert(pPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfig pipeline_config {};
    Pipeline::DefaultPipelineConfig(pipeline_config);

    pipeline_config.render_pass     = pRenderer.getSwapChainRenderPass();
    pipeline_config.pipeline_layout = pPipelineLayout;

    pPipeline =
        std::make_unique<Pipeline>(pDevice, "shaders/simple.vert.spv", "shaders/simple.frag.spv", pipeline_config);
  }

  void Application::pRenderGameObjects(VkCommandBuffer command_buffer) {
    pPipeline->Bind(command_buffer);

    for (auto &object : pGameObjects) {
      TestPushConstantData push {};

      push.offset    = object.uTransform2d.translation;
      push.transform = object.uTransform2d.getMat2();

      vkCmdPushConstants(command_buffer,
                         pPipelineLayout,
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                         0,
                         sizeof(TestPushConstantData),
                         &push);

      object.uModel->Bind(command_buffer);
      object.uModel->Draw(command_buffer);
    }
  }
}  // namespace svke
