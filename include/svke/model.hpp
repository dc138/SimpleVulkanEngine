#ifndef SVKE_MODEL_HPP
#define SVKE_MODEL_HPP

#include "defines.hpp"
#include "device.hpp"
#include "pch.hpp"

namespace svke {
  class Model {
   public:
    struct Vertex {
      glm::vec2 position;
      glm::vec3 color;

      static std::vector<VkVertexInputBindingDescription>   getBindings();
      static std::vector<VkVertexInputAttributeDescription> getAtributes();
    };

   public:
    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    Model(const Model& other) = delete;
    Model& operator=(const Model& other) = delete;

   public:
    void Bind(VkCommandBuffer buffer);
    void Draw(VkCommandBuffer buffer);

   private:
    void pCreateVertexBuffer(const std::vector<Vertex>& vertices);

   private:
    Device&        pDevice;
    VkBuffer       pVertexBuffer;
    VkDeviceMemory pVertexBufferMemory;
    uint32_t       pVertexCount;
  };
}

#endif