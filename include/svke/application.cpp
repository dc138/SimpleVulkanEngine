#include "application.hpp"
#include "defines.hpp"
#include "pch.hpp"

std::unique_ptr<svke::Model> CreateCubeModel(svke::Device& device, glm::vec3 offset) {
  std::vector<svke::Model::Vertex> vertices = {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

      // top face
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
  };

  std::vector<uint32_t> indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                                   12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

  for (auto& v : vertices) {
    v.position += offset;
  }

  return std::make_unique<svke::Model>(device, vertices, indices);
};

namespace svke {
  Application::Application(uint32_t width, uint32_t height, const std::string& window_name)
      : pWidth {width}, pHeight {height}, pWindowName {window_name} {
    pLoadGameObjects();
  }

  Application::~Application() {}

  void Application::Run() {
    pCamera.SetViewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));

    while (!pWindow.ShouldClose()) {
      glfwPollEvents();

      pCamera.UsePerspectiveProjection(glm::radians(50.f), pRenderer.getAspectRatio(), 0.1f, 10.f);

      if (auto command_buffer = pRenderer.BeginFrame()) {
        pRenderer.BeginSwapChainRenderPass(command_buffer);
        pSimpleRenderSystem.RenderGameObjects(command_buffer, pGameObjects, pCamera);
        pRenderer.EndSwapChainRenderPass(command_buffer);
        pRenderer.EndFrame();
      }
    }

    vkDeviceWaitIdle(pDevice.getDevice());
  }

  void Application::pLoadGameObjects() {
    std::shared_ptr<Model> cube_model = CreateCubeModel(pDevice, {0.0f, 0.0f, 0.0f});

    auto cube_object = GameObject::CreateGameObject();

    cube_object.ObjectModel           = cube_model;
    cube_object.Transform.translation = {0.0f, 0.0f, 2.5f};
    cube_object.Transform.scale       = {0.5f, 0.5f, 0.5f};
    cube_object.Transform.rotation    = {0.0f, 0.0f, 0.0f};

    pGameObjects.push_back(std::move(cube_object));
  }
}
