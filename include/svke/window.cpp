#include "svke/window.hpp"

#include <stdexcept>

namespace svke {
  Window::Window(uint32_t width, uint32_t height, const std::string& win_name)
      : pWidth {width}, pHeight {height}, pWindowName {win_name} {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    pWindow = glfwCreateWindow(width, height, win_name.c_str(), nullptr, nullptr);
  }

  Window::~Window() {
    glfwDestroyWindow(pWindow);
    glfwTerminate();
  }

  bool Window::ShouldClose() { return glfwWindowShouldClose(pWindow); }

  void Window::PollEvents() { glfwPollEvents(); }

  void Window::pCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, pWindow, nullptr, surface) != VK_SUCCESS) {
      throw std::runtime_error("Window surface creation failed");
    }
  }
}