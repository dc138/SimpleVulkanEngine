#ifndef SVKE_SIMPLE_RENDER_SYSTEM_HPP
#define SVKE_SIMPLE_RENDER_SYSTEM_HPP

#include "camera.hpp"
#include "defines.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pch.hpp"
#include "pipeline.hpp"

namespace svke {
  struct PushConstantData {
    glm::mat4 transform {1.0f};
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
    void RenderGameObjects(VkCommandBuffer command_buffer, std::vector<GameObject> &game_objects, const Camera &camera);

   private:
    Device &                  pDevice;
    std::unique_ptr<Pipeline> pPipeline;
    VkPipelineLayout          pPipelineLayout;
  };
}

#endif
