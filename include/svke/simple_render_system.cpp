#include "defines.hpp"
#include "pch.hpp"
#include "simple_render_system.hpp"

namespace svke {
  SimpleRenderSystem::SimpleRenderSystem(Device& device, VkRenderPass render_pass) : pDevice {device} {
    pCreatePipelineLayout();
    pCreatePipeline(render_pass);
  }

  SimpleRenderSystem::~SimpleRenderSystem() { vkDestroyPipelineLayout(pDevice.getDevice(), pPipelineLayout, nullptr); }

  void SimpleRenderSystem::pCreatePipelineLayout() {
    VkPushConstantRange push_constant_range {};

    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset     = 0;
    push_constant_range.size       = sizeof(PushConstantData);

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

  void SimpleRenderSystem::pCreatePipeline(VkRenderPass render_pass) {
    assert(pPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfig pipeline_config {};
    Pipeline::DefaultPipelineConfig(pipeline_config);

    pipeline_config.render_pass     = render_pass;
    pipeline_config.pipeline_layout = pPipelineLayout;

    pPipeline =
        std::make_unique<Pipeline>(pDevice, "shaders/simple.vert.spv", "shaders/simple.frag.spv", pipeline_config);
  }

  void SimpleRenderSystem::RenderGameObjects(VkCommandBuffer          command_buffer,
                                             std::vector<GameObject>& game_objects,
                                             const Camera&            camera) {
    pPipeline->Bind(command_buffer);

    for (auto& object : game_objects) {
      object.Transform.rotation.y = glm::mod(object.Transform.rotation.y + 0.001f, glm::two_pi<float>());
      object.Transform.rotation.x = glm::mod(object.Transform.rotation.x + 0.0005f, glm::two_pi<float>());

      PushConstantData push {};

      push.transform = (camera.getProjectionMatrix() * camera.getViewMatrix()) * object.Transform.matrix();

      vkCmdPushConstants(command_buffer,
                         pPipelineLayout,
                         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                         0,
                         sizeof(PushConstantData),
                         &push);

      object.ObjectModel->Bind(command_buffer);
      object.ObjectModel->Draw(command_buffer);
    }
  }
}
