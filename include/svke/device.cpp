#if defined(NDEBUG) || defined(_DEBUG)
#define SVKE_DEBUG
#endif

#define UNUSED(x) (void(x))

#include "device.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <unordered_set>

namespace svke {
  static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                      VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                      const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                      void *user_data) {
    UNUSED(message_severity);
    UNUSED(message_type);
    UNUSED(user_data);

    std::cerr << "\n\033[1m\033[31mValidation Layer:\033[0m " << callback_data->pMessage << "\n\n";
    return VK_FALSE;
  }

  VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info,
                                        const VkAllocationCallbacks *allocator,
                                        VkDebugUtilsMessengerEXT *debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
      return func(instance, create_info, allocator, debug_messenger);

    } else {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
  }

  void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
                                     const VkAllocationCallbacks *allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
      func(instance, debug_messenger, allocator);
    }
  }

  Device::Device(Window &window) : pWindow {window} {
    pCreateInstance();

#ifdef SVKE_DEBUG
    pSetupDebugMessenger();
#endif

    pCreateSurface();
    pPickPhysicalDevice();
    pCreateLogicalDevice();
    pCreateCommandPool();
  }

  Device::~Device() {
    vkDestroyCommandPool(pDevice, pCommandPool, nullptr);
    vkDestroyDevice(pDevice, nullptr);

#ifdef SVKE_DEBUG
    DestroyDebugUtilsMessengerEXT(pInstance, pDebugMessenger, nullptr);
#endif

    vkDestroySurfaceKHR(pInstance, pSurface, nullptr);
    vkDestroyInstance(pInstance, nullptr);
  }

  void Device::pCreateInstance() {
#ifdef SVKE_DEBUG
    if (!pCheckValidationLayerSupport()) {
      throw std::runtime_error("Validation layers requested, but not available");
    }
#endif

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "SimpleVulkanEngine Application";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "SimpleVulkanEngine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    auto extensions = getRequiredExtensions();
    create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

#ifdef SVKE_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;

    create_info.enabledLayerCount = static_cast<uint32_t>(pValidationLayers.size());
    create_info.ppEnabledLayerNames = pValidationLayers.data();

    pPopulateDebugMessengerCreateInfo(debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
#else
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
#endif

    if (vkCreateInstance(&create_info, nullptr, &pInstance) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create instance");
    }

    pHasGflwRequiredInstanceExtensions();
  }

  void Device::pPickPhysicalDevice() {
    uint32_t device_count = 0;

    vkEnumeratePhysicalDevices(pInstance, &device_count, nullptr);

    if (device_count == 0) {
      throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

#ifdef SVKE_VERBOSE_DEVICE
    std::cout << "Device count: " << device_count << std::endl;
#endif

    std::vector<VkPhysicalDevice> devices(device_count);

    vkEnumeratePhysicalDevices(pInstance, &device_count, devices.data());

    for (const auto &device : devices) {
      if (pDeviceSuitable(device)) {
        pPhysicalDevice = device;
        break;
      }
    }

    if (pPhysicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("Failed to find a suitable GPU");
    }

    vkGetPhysicalDeviceProperties(pPhysicalDevice, &properties);

#ifdef SVKE_VERBOSE_DEVICE
    std::cout << "Physical device: " << properties.deviceName << std::endl;
#endif
  }

  void Device::pCreateLogicalDevice() {
    QueueFamilyIndices indices = pFindQueueFamilies(pPhysicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = {indices.graphics_family, indices.present_family};

    float queue_priority = 1.0f;

    for (uint32_t queue_family : unique_queue_families) {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queue_family;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queue_priority;
      queue_create_infos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pQueueCreateInfos = queue_create_infos.data();

    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = static_cast<uint32_t>(pDeviceExtensions.size());
    create_info.ppEnabledExtensionNames = pDeviceExtensions.data();

#ifdef SVKE_DEBUG
    create_info.enabledLayerCount = static_cast<uint32_t>(pValidationLayers.size());
    create_info.ppEnabledLayerNames = pValidationLayers.data();
#else
    create_info.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(pPhysicalDevice, &create_info, nullptr, &pDevice) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(pDevice, indices.graphics_family, 0, &pGraphicsQueue);
    vkGetDeviceQueue(pDevice, indices.present_family, 0, &pPresentQueue);
  }

  void Device::pCreateCommandPool() {
    QueueFamilyIndices queue_family_indices = FindPhysicalQueueFamilies();

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(pDevice, &pool_info, nullptr, &pCommandPool) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create command pool");
    }
  }

  void Device::pCreateSurface() { pWindow.pCreateWindowSurface(pInstance, &pSurface); }

  bool Device::pDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = pFindQueueFamilies(device);

    bool extensions_supported = pCheckDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensions_supported) {
      SwapChainSupportDetails swapChainSupport = pQuerySwapChainSupport(device);
      swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.IsComplete() && extensions_supported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
  }

  void Device::pPopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &create_info) {
    create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;
    create_info.pUserData = nullptr;
  }

  void Device::pSetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    pPopulateDebugMessengerCreateInfo(create_info);

    if (CreateDebugUtilsMessengerEXT(pInstance, &create_info, nullptr, &pDebugMessenger) != VK_SUCCESS) {
      throw std::runtime_error("Failed to set up debug messenger");
    }
  }

  bool Device::pCheckValidationLayerSupport() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char *layer_name : pValidationLayers) {
      bool found = false;

      for (const auto &layer_properties : available_layers) {
        if (strcmp(layer_name, layer_properties.layerName) == 0) {
          found = true;
          break;
        }
      }

      if (!found) {
        return false;
      }
    }

    return true;
  }

  std::vector<const char *> Device::getRequiredExtensions() {
    uint32_t glfw_extension_count = 0;
    const char **glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    std::vector<const char *> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

#ifdef SVKE_DEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
  }

  void Device::pHasGflwRequiredInstanceExtensions() {
    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> extensions(extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

#ifdef SVKE_VERBOSE_DEVICE
    std::cout << "Available extensions:" << std::endl;
#endif

    std::unordered_set<std::string> available;

    for (const auto &extension : extensions) {
#ifdef SVKE_VERBOSE_DEVICE
      std::cout << "\t" << extension.extensionName << std::endl;
#endif
      available.insert(extension.extensionName);
    }

#ifdef SVKE_VERBOSE_DEVICE
    std::cout << "Required extensions:" << std::endl;
#endif
    auto required_extensions = getRequiredExtensions();

    for (const auto &required : required_extensions) {
#ifdef SVKE_VERBOSE_DEVICE
      std::cout << "\t" << required << std::endl;
#endif
      if (available.find(required) == available.end()) {
        throw std::runtime_error("Missing required glfw extension");
      }
    }
  }

  bool Device::pCheckDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(pDeviceExtensions.begin(), pDeviceExtensions.end());

    for (const auto &extension : available_extensions) {
      required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
  }

  QueueFamilyIndices Device::pFindQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    int i = 0;
    for (const auto &queue_family : queue_families) {
      if (queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
        indices.graphics_family = i;
        indices.graphics_family_has_value = true;
      }
      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(device, i, pSurface, &presentSupport);
      if (queue_family.queueCount > 0 && presentSupport) {
        indices.present_family = i;
        indices.present_family_has_value = true;
      }
      if (indices.IsComplete()) {
        break;
      }

      i++;
    }

    return indices;
  }

  SwapChainSupportDetails Device::pQuerySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, pSurface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, pSurface, &format_count, nullptr);

    if (format_count != 0) {
      details.formats.resize(format_count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device, pSurface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, pSurface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
      details.present_modes.resize(present_mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device, pSurface, &present_mode_count, details.present_modes.data());
    }
    return details;
  }

  VkFormat Device::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                       VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(pPhysicalDevice, format, &props);

      if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
        return format;
      } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
        return format;
      }
    }
    throw std::runtime_error("Failed to find supported format");
  }

  uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(pPhysicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
      if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }

    throw std::runtime_error("Failed to find suitable memory type");
  }

  void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                            VkBuffer &buffer, VkDeviceMemory &bufferMemory) {
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(pDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(pDevice, buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(pDevice, &alloc_info, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(pDevice, buffer, bufferMemory, 0);
  }

  VkCommandBuffer Device::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = pCommandPool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(pDevice, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
  }

  void Device::EndSingleTimeCommands(VkCommandBuffer command_buffer) {
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(pGraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(pGraphicsQueue);

    vkFreeCommandBuffers(pDevice, pCommandPool, 1, &command_buffer);
  }

  void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer command_buffer = BeginSingleTimeCommands();

    VkBufferCopy copy_region {};
    copy_region.srcOffset = 0;  // Optional
    copy_region.dstOffset = 0;  // Optional
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copy_region);

    EndSingleTimeCommands(command_buffer);
  }

  void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height,
                                 uint32_t layer_count) {
    VkCommandBuffer command_buffer = BeginSingleTimeCommands();

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layer_count;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommands(command_buffer);
  }

  void Device::CreateImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image,
                                   VkDeviceMemory &imageMemory) {
    if (vkCreateImage(pDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create image");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(pDevice, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(pDevice, &alloc_info, nullptr, &imageMemory) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate image memory");
    }

    if (vkBindImageMemory(pDevice, image, imageMemory, 0) != VK_SUCCESS) {
      throw std::runtime_error("Failed to bind image memory");
    }
  }
}