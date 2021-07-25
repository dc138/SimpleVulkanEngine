#ifndef SVKE_SIMPLE_RENDER_SYSTEM_HPP
#define SVKE_SIMPLE_RENDER_SYSTEM_HPP

#include "defines.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pch.hpp"
#include "pipeline.hpp"

namespace svke {
  struct PushConstantData {
    glm::mat2 transform {1.0f};
    glm::vec2 offset {0.0f, 0.0f};
  };

  class SimpleRenderSystem {
   public:
    SimpleRenderSystem(Device &device, VkRenderPass render_pass);
    ~SimpleRenderSystem();

    SimpleRenderSystem(const SimpleRenderSystem &other) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem &other) = delete;

   private:
    void pCreatePipelineLayout();
    void pCreatePipeline(VkRenderPass render_pass);

   public:
    void RenderGameObjects(VkCommandBuffer command_buffer, std::vector<GameObject> &game_objects);

   private:
    Device &                  pDevice;
    std::unique_ptr<Pipeline> pPipeline;
    VkPipelineLayout          pPipelineLayout;
  };
}

#endif
