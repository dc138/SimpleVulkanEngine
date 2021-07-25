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
  }

  Application::~Application() {}

  void Application::Run() {
    while (!pWindow.ShouldClose()) {
      glfwPollEvents();

      if (auto command_buffer = pRenderer.BeginFrame()) {
        pRenderer.BeginSwapChainRenderPass(command_buffer);
        pSimpleRenderSystem.RenderGameObjects(command_buffer, pGameObjects);
        pRenderer.EndSwapChainRenderPass(command_buffer);
        pRenderer.EndFrame();
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

    auto triangle                            = GameObject::CreateGameObject();
    triangle.ObjectModel                     = model;
    triangle.ObjectTransform2d.translation.x = .2f;
    triangle.ObjectTransform2d.translation.y = -.3f;
    triangle.ObjectTransform2d.scale         = {.5f, 1.5f};
    triangle.ObjectTransform2d.rotation      = .25f * glm::two_pi<float>();

    pGameObjects.push_back(std::move(triangle));
  }
}
