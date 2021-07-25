#include "defines.hpp"
#include "pch.hpp"
#include "swap_chain.hpp"

namespace svke {
  SwapChain::SwapChain(Device &device, VkExtent2D window_extent) : pDevice {device}, pWindowExtent {window_extent} {
    pInitSwapChain();
  }

  SwapChain::SwapChain(Device &device, VkExtent2D window_extent, std::shared_ptr<SwapChain> previous)
      : pDevice {device}, pWindowExtent {window_extent}, pOldSwapChain {previous} {
    pInitSwapChain();

    pOldSwapChain = nullptr;
  }

  void SwapChain::pInitSwapChain() {
    pCreateSwapChain();
    pCreateImageViews();
    pCreateRenderPass();
    pCreateDepthResources();
    pCreateFramebuffers();
    pCreateSyncObjects();
  }

  SwapChain::~SwapChain() {
    for (auto image_view : pSwapChainImageViews) {
      vkDestroyImageView(pDevice.getDevice(), image_view, nullptr);
    }

    pSwapChainImageViews.clear();

    if (pSwapChain != nullptr) {
      vkDestroySwapchainKHR(pDevice.getDevice(), pSwapChain, nullptr);
      pSwapChain = nullptr;
    }

    for (uint64_t i = 0; i < pDepthImages.size(); i++) {
      vkDestroyImageView(pDevice.getDevice(), pDepthImageViews[i], nullptr);
      vkDestroyImage(pDevice.getDevice(), pDepthImages[i], nullptr);
      vkFreeMemory(pDevice.getDevice(), pDepthImageMemorys[i], nullptr);
    }

    for (auto framebuffer : pSwapChainFramebuffers) {
      vkDestroyFramebuffer(pDevice.getDevice(), framebuffer, nullptr);
    }

    vkDestroyRenderPass(pDevice.getDevice(), pRenderPass, nullptr);

    for (uint64_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(pDevice.getDevice(), pRenderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(pDevice.getDevice(), pImageAvailableSemaphores[i], nullptr);
      vkDestroyFence(pDevice.getDevice(), pInFlightFences[i], nullptr);
    }
  }

  VkResult SwapChain::AcquireNextImage(uint32_t *imageIndex) {
    vkWaitForFences(
        pDevice.getDevice(), 1, &pInFlightFences[pCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    return vkAcquireNextImageKHR(pDevice.getDevice(),
                                 pSwapChain,
                                 std::numeric_limits<uint64_t>::max(),
                                 pImageAvailableSemaphores[pCurrentFrame],  // must be a not signaled semaphore
                                 VK_NULL_HANDLE,
                                 imageIndex);
  }

  VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) {
    if (pInFlightImages[*imageIndex] != VK_NULL_HANDLE) {
      vkWaitForFences(pDevice.getDevice(), 1, &pInFlightImages[*imageIndex], VK_TRUE, UINT64_MAX);
    }

    pInFlightImages[*imageIndex] = pInFlightFences[pCurrentFrame];

    VkSubmitInfo submit_info = {};
    submit_info.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore          wait_semaphores[] = {pImageAvailableSemaphores[pCurrentFrame]};
    VkPipelineStageFlags wait_stages[]     = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores    = wait_semaphores;
    submit_info.pWaitDstStageMask  = wait_stages;

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = buffers;

    VkSemaphore signal_semaphores[]  = {pRenderFinishedSemaphores[pCurrentFrame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    vkResetFences(pDevice.getDevice(), 1, &pInFlightFences[pCurrentFrame]);
    if (vkQueueSubmit(pDevice.getGraphicsQueue(), 1, &submit_info, pInFlightFences[pCurrentFrame]) != VK_SUCCESS) {
      throw std::runtime_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {pSwapChain};
    present_info.swapchainCount  = 1;
    present_info.pSwapchains     = swap_chains;

    present_info.pImageIndices = imageIndex;

    auto result = vkQueuePresentKHR(pDevice.getPresentQueue(), &present_info);

    pCurrentFrame = (pCurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
  }

  void SwapChain::pCreateSwapChain() {
    SwapChainSupportDetails swapChainSupport = pDevice.getSwapChainSupport();

    VkSurfaceFormatKHR surface_format = pChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR   present_mode   = pChooseSwapPresentMode(swapChainSupport.present_modes);
    VkExtent2D         window_extent  = pChooseSwapExtent(swapChainSupport.capabilities);

    uint32_t image_count = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && image_count > swapChainSupport.capabilities.maxImageCount) {
      image_count = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface                  = pDevice.getSurface();

    create_info.minImageCount    = image_count;
    create_info.imageFormat      = surface_format.format;
    create_info.imageColorSpace  = surface_format.colorSpace;
    create_info.imageExtent      = window_extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices              = pDevice.FindPhysicalQueueFamilies();
    uint32_t           queueFamilyIndices[] = {indices.graphics_family, indices.present_family};

    if (indices.graphics_family != indices.present_family) {
      create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices   = queueFamilyIndices;

    } else {
      create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
      create_info.queueFamilyIndexCount = 0;        // Optional
      create_info.pQueueFamilyIndices   = nullptr;  // Optional
    }

    create_info.preTransform   = swapChainSupport.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    create_info.presentMode = present_mode;
    create_info.clipped     = VK_TRUE;

    create_info.oldSwapchain = pOldSwapChain == nullptr ? VK_NULL_HANDLE : pOldSwapChain->pSwapChain;

    if (vkCreateSwapchainKHR(pDevice.getDevice(), &create_info, nullptr, &pSwapChain) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create swap chain");
    }

    // we only specified a minimum number of images in the swap chain, so the implementation is
    // allowed to create a swap chain with more. That's why we'll first query the final number of
    // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
    // retrieve the handles.
    vkGetSwapchainImagesKHR(pDevice.getDevice(), pSwapChain, &image_count, nullptr);
    pSwapChainImages.resize(image_count);
    vkGetSwapchainImagesKHR(pDevice.getDevice(), pSwapChain, &image_count, pSwapChainImages.data());

    pSwapChainImageFormat = surface_format.format;
    pSwapChainExtent      = window_extent;
  }

  void SwapChain::pCreateImageViews() {
    pSwapChainImageViews.resize(pSwapChainImages.size());

    for (uint64_t i = 0; i < pSwapChainImages.size(); i++) {
      VkImageViewCreateInfo view_info {};

      view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_info.image                           = pSwapChainImages[i];
      view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      view_info.format                          = pSwapChainImageFormat;
      view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      view_info.subresourceRange.baseMipLevel   = 0;
      view_info.subresourceRange.levelCount     = 1;
      view_info.subresourceRange.baseArrayLayer = 0;
      view_info.subresourceRange.layerCount     = 1;

      if (vkCreateImageView(pDevice.getDevice(), &view_info, nullptr, &pSwapChainImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view");
      }
    }
  }

  void SwapChain::pCreateRenderPass() {
    VkAttachmentDescription depth_attachment {};

    depth_attachment.format         = FindDepthFormat();
    depth_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref {};

    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription color_attachment = {};

    color_attachment.format         = getSwapChainImageFormat();
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};

    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};

    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};

    dependency.dstSubpass    = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    std::array<VkAttachmentDescription, 2> attachments      = {color_attachment, depth_attachment};
    VkRenderPassCreateInfo                 render_pass_info = {};

    render_pass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments    = attachments.data();
    render_pass_info.subpassCount    = 1;
    render_pass_info.pSubpasses      = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies   = &dependency;

    if (vkCreateRenderPass(pDevice.getDevice(), &render_pass_info, nullptr, &pRenderPass) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create render pass");
    }
  }

  void SwapChain::pCreateFramebuffers() {
    pSwapChainFramebuffers.resize(getImageCount());

    for (uint64_t i = 0; i < getImageCount(); i++) {
      std::array<VkImageView, 2> attachments = {pSwapChainImageViews[i], pDepthImageViews[i]};

      VkExtent2D              swap_chain_extent = getSwapChainExtent();
      VkFramebufferCreateInfo framebuffer_info  = {};

      framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebuffer_info.renderPass      = pRenderPass;
      framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
      framebuffer_info.pAttachments    = attachments.data();
      framebuffer_info.width           = swap_chain_extent.width;
      framebuffer_info.height          = swap_chain_extent.height;
      framebuffer_info.layers          = 1;

      if (vkCreateFramebuffer(pDevice.getDevice(), &framebuffer_info, nullptr, &pSwapChainFramebuffers[i]) !=
          VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer");
      }
    }
  }

  void SwapChain::pCreateDepthResources() {
    VkFormat   depthFormat       = FindDepthFormat();
    VkExtent2D swap_chain_extent = getSwapChainExtent();

    pDepthImages.resize(getImageCount());
    pDepthImageMemorys.resize(getImageCount());
    pDepthImageViews.resize(getImageCount());

    for (uint64_t i = 0; i < pDepthImages.size(); i++) {
      VkImageCreateInfo image_info {};

      image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
      image_info.imageType     = VK_IMAGE_TYPE_2D;
      image_info.extent.width  = swap_chain_extent.width;
      image_info.extent.height = swap_chain_extent.height;
      image_info.extent.depth  = 1;
      image_info.mipLevels     = 1;
      image_info.arrayLayers   = 1;
      image_info.format        = depthFormat;
      image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
      image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      image_info.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
      image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
      image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
      image_info.flags         = 0;

      pDevice.CreateImageWithInfo(
          image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pDepthImages[i], pDepthImageMemorys[i]);

      VkImageViewCreateInfo view_info {};

      view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      view_info.image                           = pDepthImages[i];
      view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      view_info.format                          = depthFormat;
      view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
      view_info.subresourceRange.baseMipLevel   = 0;
      view_info.subresourceRange.levelCount     = 1;
      view_info.subresourceRange.baseArrayLayer = 0;
      view_info.subresourceRange.layerCount     = 1;

      if (vkCreateImageView(pDevice.getDevice(), &view_info, nullptr, &pDepthImageViews[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view");
      }
    }
  }

  void SwapChain::pCreateSyncObjects() {
    pImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    pRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    pInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    pInFlightImages.resize(getImageCount(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint64_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(pDevice.getDevice(), &semaphore_info, nullptr, &pImageAvailableSemaphores[i]) !=
              VK_SUCCESS ||
          vkCreateSemaphore(pDevice.getDevice(), &semaphore_info, nullptr, &pRenderFinishedSemaphores[i]) !=
              VK_SUCCESS ||
          vkCreateFence(pDevice.getDevice(), &fence_info, nullptr, &pInFlightFences[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create synchronization objects for a frame");
      }
    }
  }

  VkSurfaceFormatKHR SwapChain::pChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
    for (const auto &available_format : availableFormats) {
      if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
          available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
        return available_format;
      }
    }

    return availableFormats[0];
  }

  VkPresentModeKHR SwapChain::pChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
    for (const auto &available_present_mode : availablePresentModes) {
      if (available_present_mode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
#ifdef SVKE_VERBOSE_PRESENT_MODE
        std::cout << "Present mode: Immediate" << std::endl;
#endif
        return available_present_mode;
      }
    }

    for (const auto &available_present_mode : availablePresentModes) {
      if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
#ifdef SVKE_VERBOSE_PRESENT_MODE
        std::cout << "Present mode: Mailbox" << std::endl;
#endif
        return available_present_mode;
      }
    }

#ifdef SVKE_VERBOSE_PRESENT_MODE
    std::cout << "Present mode: V-Sync" << std::endl;
#endif
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D SwapChain::pChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
      return capabilities.currentExtent;

    } else {
      VkExtent2D actual_extent = pWindowExtent;

      actual_extent.width =
          std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actual_extent.width));
      actual_extent.height = std::max(capabilities.minImageExtent.height,
                                      std::min(capabilities.maxImageExtent.height, actual_extent.height));

      return actual_extent;
    }
  }

  VkFormat SwapChain::FindDepthFormat() {
    return pDevice.FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
  }
}
