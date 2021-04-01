#ifndef SVKE_WINDOW_HPP
#define SVKE_WINDOW_HPP

#include "defines.hpp"
#include "pch.hpp"

namespace svke {
  class Window {
   public:
    Window(uint32_t width, uint32_t height, const std::string& win_name);
    ~Window();

    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

    bool ShouldClose();
    void PollEvents();

    VkExtent2D getExtent() { return {static_cast<uint32_t>(pWidth), static_cast<uint32_t>(pHeight)}; }
    bool       WasResized() { return pFrameBufferResized; }
    void       ResetResize() { pFrameBufferResized = false; }

    friend class Device;

   private:
    void pCreateWindow();
    void pCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

   private:
    static void pFramebufferResizeCallback(GLFWwindow* window, int32_t width, int32_t height);

   private:
    uint32_t pWidth;
    uint32_t pHeight;
    bool     pFrameBufferResized = false;

    GLFWwindow* pWindow;
    std::string pWindowName;
  };
}

#endif