#ifndef SVKE_SWAP_CHAIN_HPP
#define SVKE_SWAP_CHAIN_HPP

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "device.hpp"

#define MAX_FRAMES_IN_FLIGHT 2

namespace svke {
  class SwapChain {
   public:
    SwapChain(Device &device, VkExtent2D window_extent);
    ~SwapChain();

    SwapChain(const SwapChain &other) = delete;
    void operator=(const SwapChain &other) = delete;

   public:
    VkFramebuffer getFrameBuffer(int index) { return pSwapChainFramebuffers[index]; }
    VkRenderPass  getRenderPass() { return pRenderPass; }
    VkImageView   getImageView(int index) { return pSwapChainImageViews[index]; }
    uint64_t      getImageCount() { return pSwapChainImages.size(); }
    VkFormat      getSwapChainImageFormat() { return pSwapChainImageFormat; }
    VkExtent2D    getSwapChainExtent() { return pSwapChainExtent; }
    uint32_t      getWidth() { return pSwapChainExtent.width; }
    uint32_t      getHeight() { return pSwapChainExtent.height; }

   public:
    float getExtentAspectRatio() {
      return static_cast<float>(pSwapChainExtent.width) / static_cast<float>(pSwapChainExtent.height);
    }

   public:
    VkFormat FindDepthFormat();
    VkResult AcquireNextImage(uint32_t *image_index);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *image_index);

   private:
    void pCreateSwapChain();
    void pCreateImageViews();
    void pCreateDepthResources();
    void pCreateRenderPass();
    void pCreateFramebuffers();
    void pCreateSyncObjects();

   private:
    VkSurfaceFormatKHR pChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR   pChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D         pChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

   private:
    VkFormat   pSwapChainImageFormat;
    VkExtent2D pSwapChainExtent;

   private:
    std::vector<VkFramebuffer> pSwapChainFramebuffers;
    VkRenderPass               pRenderPass;

   private:
    std::vector<VkImage>        pDepthImages;
    std::vector<VkDeviceMemory> pDepthImageMemorys;
    std::vector<VkImageView>    pDepthImageViews;
    std::vector<VkImage>        pSwapChainImages;
    std::vector<VkImageView>    pSwapChainImageViews;

   private:
    Device &       pDevice;
    VkExtent2D     pWindowExtent;
    VkSwapchainKHR pSwapChain;

   private:
    std::vector<VkSemaphore> pImageAvailableSemaphores;
    std::vector<VkSemaphore> pRenderFinishedSemaphores;
    std::vector<VkFence>     pInFlightFences;
    std::vector<VkFence>     pInFlightImages;
    uint64_t                 pCurrentFrame = 0;
  };
}

#endif