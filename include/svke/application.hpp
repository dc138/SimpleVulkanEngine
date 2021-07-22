#ifndef SVKE_APPLICATION_HPP
#define SVKE_APPLICATION_HPP

#include "defines.hpp"
#include "device.hpp"
#include "game_object.hpp"
#include "pch.hpp"
#include "renderer.hpp"
#include "simple_render_system.hpp"
#include "window.hpp"

namespace svke {
  class Application {
   public:
    Application(uint32_t width, uint32_t height, const std::string &window_name);
    ~Application();

    Application(const Application &other) = delete;
    Application &operator=(const Application &other) = delete;

   public:
    void Run();

   private:
    void pLoadGameObjects();

   private:
    uint32_t    pWidth;
    uint32_t    pHeight;
    std::string pWindowName;

   private:
    Window                  pWindow {pWidth, pHeight, pWindowName};
    Device                  pDevice {pWindow};
    Renderer                pRenderer {pWindow, pDevice};
    SimpleRenderSystem      pSimpleRenderSystem {pDevice, pRenderer.getSwapChainRenderPass()};
    std::vector<GameObject> pGameObjects;
  };
}

#endif
