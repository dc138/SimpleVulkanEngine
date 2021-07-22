#include "defines.hpp"
#include "pch.hpp"
#include "renderer.hpp"

namespace svke {
  Renderer::Renderer(Window& window, Device& device) : pWindow {window}, pDevice {device} {
    pRecreateSwapChain();
    pCreateCommandBuffers();
  }

  Renderer::~Renderer() { pFreeCommandBuffers(); }

  void Renderer::pCreateCommandBuffers() {
    pCommandBuffer.resize(pSwapChain->getImageCount());

    VkCommandBufferAllocateInfo alloc_info {};

    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool        = pDevice.getCommandPool();
    alloc_info.commandBufferCount = static_cast<uint32_t>(pCommandBuffer.size());

    if (vkAllocateCommandBuffers(pDevice.getDevice(), &alloc_info, pCommandBuffer.data()) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate command buffers");
    }
  }

  void Renderer::pFreeCommandBuffers() {
    vkFreeCommandBuffers(pDevice.getDevice(),
                         pDevice.getCommandPool(),
                         static_cast<uint32_t>(pCommandBuffer.size()),
                         pCommandBuffer.data());
    pCommandBuffer.clear();
  }

  void Renderer::pRecreateSwapChain() {
    auto extent = pWindow.getExtent();

    while (extent.width == 0 || extent.height == 0) {
      extent = pWindow.getExtent();
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(pDevice.getDevice());

    if (pSwapChain == nullptr) {
      pSwapChain = std::make_unique<SwapChain>(pDevice, extent);
    } else {
      pSwapChain = std::make_unique<SwapChain>(pDevice, extent, std::move(pSwapChain));

      if (pSwapChain->getImageCount() != pCommandBuffer.size()) {
        pFreeCommandBuffers();
        pCreateCommandBuffers();
      }
    }

    // COME BACK HERE
  }

  VkCommandBuffer Renderer::uBeginFrame() {
    assert(!pIsFrameStarted && "Cannot begin frame with one already in progress");

    VkResult result = pSwapChain->AcquireNextImage(&pCurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
      pRecreateSwapChain();
      return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
      throw std::runtime_error("Failed to acquire swap chain image");
    }

    pIsFrameStarted = true;

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(pCommandBuffer[pCurrentImageIndex], &begin_info) != VK_SUCCESS) {
      throw std::runtime_error("Failed to begin recording command buffer");
    }

    return pCommandBuffer[pCurrentImageIndex];
  }

  void Renderer::uEndFrame() {
    assert(pIsFrameStarted && "Cannot end a frame without one being started");

    if (vkEndCommandBuffer(pCommandBuffer[pCurrentImageIndex]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to record command buffer");
    }

    auto result = pSwapChain->SubmitCommandBuffers(&pCommandBuffer[pCurrentImageIndex], &pCurrentImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || pWindow.WasResized()) {
      pWindow.ResetResize();
      pRecreateSwapChain();
    } else if (result != VK_SUCCESS) {
      throw std::runtime_error("Failed to present swap chain image");
    }

    pIsFrameStarted = false;
  }

  void Renderer::uBeginSwapChainRenderPass(VkCommandBuffer command_buffer) {
    assert(pIsFrameStarted && "Cannot begin a swapchain render pass when no frame is in progress");
    assert(command_buffer == pCommandBuffer[pCurrentImageIndex] &&
           "Cannot begin swapchain render pass on commadn buffer of a different frame");

    VkRenderPassBeginInfo render_pass_info {};

    render_pass_info.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass  = pSwapChain->getRenderPass();
    render_pass_info.framebuffer = pSwapChain->getFrameBuffer(pCurrentImageIndex);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = pSwapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clear_values {};

    clear_values[0].color            = {{0.1f, 0.1f, 0.1f, 1.0f}};
    clear_values[1].depthStencil     = {1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues    = clear_values.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport {};

    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(pSwapChain->getSwapChainExtent().width);
    viewport.height   = static_cast<float>(pSwapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor {};

    scissor.offset = {0, 0};
    scissor.extent = pSwapChain->getSwapChainExtent();

    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
  }

  void Renderer::uEndSwapChainRenderPass(VkCommandBuffer command_buffer) {
    assert(pIsFrameStarted && "Cannot end a swapchain render pass when no frame is in progress");
    assert(command_buffer == pCommandBuffer[pCurrentImageIndex] &&
           "Cannot end swapchain render pass on commadn buffer of a different frame");

    vkCmdEndRenderPass(command_buffer);
  }
}
