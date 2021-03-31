#ifndef SVKE_APPLICATION_HPP
#define SVKE_APPLICATION_HPP

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "device.hpp"
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
    uint32_t    pWidth;
    uint32_t    pHeight;
    std::string pWindowName;

   private:
    Window                       pWindow {pWidth, pHeight, pWindowName};
    Device                       pDevice {pWindow};
    SwapChain                    pSwapChain {pDevice, pWindow.getExtent()};
    std::unique_ptr<Pipeline>    pPipeline;
    VkPipelineLayout             pPipelineLayout;
    std::vector<VkCommandBuffer> pCommandBuffer;
  };
}

#endif