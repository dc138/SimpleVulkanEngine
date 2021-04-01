#ifndef SVKE_APPLICATION_HPP
#define SVKE_APPLICATION_HPP

#include "device.hpp"
#include "model.hpp"
#include "pch.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

namespace svke {
  class Application {
   public:
    Application(uint32_t width, uint32_t height, const std::string& window_name);
    ~Application();

    Application(const Application& other) = delete;
    Application& operator=(const Application& other) = delete;

   public:
    void Run();

   private:
    void pLoadModels();
    void pCreatePipelineLayout();
    void pCreatePipeline();
    void pCreateCommandBuffers();
    void pFreeCommandBuffers();
    void pDrawFrame();

   private:
    void pRecreateSwapChain();
    void pRecordCommandBuffer(uint32_t image_index);

   private:
    uint32_t    pWidth;
    uint32_t    pHeight;
    std::string pWindowName;

   private:
    Window                       pWindow {pWidth, pHeight, pWindowName};
    Device                       pDevice {pWindow};
    std::unique_ptr<SwapChain>   pSwapChain;
    std::unique_ptr<Pipeline>    pPipeline;
    VkPipelineLayout             pPipelineLayout;
    std::vector<VkCommandBuffer> pCommandBuffer;
    std::unique_ptr<Model>       pModel;
  };
}

#endif