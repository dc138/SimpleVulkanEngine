#ifndef SVKE_DEVICE_HPP
#define SVKE_DEVICE_HPP

#include <string>
#include <vector>

#include "window.hpp"

namespace svke {
  struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
  };

  struct QueueFamilyIndices {
    uint32_t graphics_family;
    uint32_t present_family;
    bool graphics_family_has_value = false;
    bool present_family_has_value = false;
    bool IsComplete() { return graphics_family_has_value && present_family_has_value; }
  };

  class Device {
   public:
    Device(Window &window);
    ~Device();

    Device(const Device &) = delete;
    void operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

   public:
    VkCommandPool getCommandPool() { return pCommandPool; }
    VkDevice getDevice() { return pDevice; }
    VkSurfaceKHR getSurface() { return pSurface; }
    VkQueue getGraphicsQueue() { return pGraphicsQueue; }
    VkQueue getPresentQueue() { return pPresentQueue; }

   public:
    SwapChainSupportDetails getSwapChainSupport() { return pQuerySwapChainSupport(pPhysicalDevice); }
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices FindPhysicalQueueFamilies() { return pFindQueueFamilies(pPhysicalDevice); }
    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

   public:
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
                      VkDeviceMemory &buffer_memory);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer command_buffer);
    void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_uffer, VkDeviceSize size);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layer_count);

    void CreateImageWithInfo(const VkImageCreateInfo &image_info, VkMemoryPropertyFlags properties, VkImage &image,
                             VkDeviceMemory &image_memory);

    VkPhysicalDeviceProperties properties;

   private:
    void pCreateInstance();
    void pSetupDebugMessenger();
    void pCreateSurface();
    void pPickPhysicalDevice();
    void pCreateLogicalDevice();
    void pCreateCommandPool();

   private:
    bool pDeviceSuitable(VkPhysicalDevice device);
    std::vector<const char *> getRequiredExtensions();
    bool pCheckValidationLayerSupport();
    QueueFamilyIndices pFindQueueFamilies(VkPhysicalDevice device);
    void pPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info);
    void pHasGflwRequiredInstanceExtensions();
    bool pCheckDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails pQuerySwapChainSupport(VkPhysicalDevice device);

   private:
    VkInstance pInstance;
    VkDebugUtilsMessengerEXT pDebugMessenger;
    VkPhysicalDevice pPhysicalDevice = VK_NULL_HANDLE;
    Window &pWindow;
    VkCommandPool pCommandPool;

   private:
    VkDevice pDevice;
    VkSurfaceKHR pSurface;
    VkQueue pGraphicsQueue;
    VkQueue pPresentQueue;

   private:
    const std::vector<const char *> pValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> pDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  };

}

#endif