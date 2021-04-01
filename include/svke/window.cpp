#include "svke/window.hpp"

#include "defines.hpp"
#include "pch.hpp"

namespace svke {
  Window::Window(uint32_t width, uint32_t height, const std::string& win_name)
      : pWidth {width}, pHeight {height}, pWindowName {win_name} {
    pCreateWindow();
  }

  Window::~Window() {
    glfwDestroyWindow(pWindow);
    glfwTerminate();
  }

  bool Window::ShouldClose() { return glfwWindowShouldClose(pWindow); }

  void Window::PollEvents() { glfwPollEvents(); }

  void Window::pCreateWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    pWindow = glfwCreateWindow(pWidth, pHeight, pWindowName.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(pWindow, this);
    glfwSetFramebufferSizeCallback(pWindow, pFramebufferResizeCallback);
  }

  void Window::pCreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, pWindow, nullptr, surface) != VK_SUCCESS) {
      throw std::runtime_error("Window surface creation failed");
    }
  }

  void Window::pFramebufferResizeCallback(GLFWwindow* window, int32_t width, int32_t height) {
    Window* svke_window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

    svke_window->pFrameBufferResized = true;
    svke_window->pWidth              = width;
    svke_window->pHeight             = height;
  }
}