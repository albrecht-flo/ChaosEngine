#pragma once

#include <vulkan/vulkan.h>

#include <optional>

#include "VulkanInstance.h"

/* Stores the required queues. */
struct QueueFamilyIndices {
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;

  bool isComplete() {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }
};

/* Stores relevant swapchain information*/
struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

/* Wrapper for the logical device and handles the creation of it for a physical device.
	Contains physical device selection.
	*/
class VulkanDevice {
private:
  VulkanDevice(VulkanInstance &instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice,
               VkDevice device, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily,
               VkQueue graphicsQueue, VkQueue presentQueue, VkPhysicalDeviceProperties properties);

  void destroy();

public:
  static const std::vector<const char *> deviceExtensions;
public:
  ~VulkanDevice();

  VulkanDevice(const VulkanDevice &o) = delete;

  VulkanDevice &operator=(const VulkanDevice &o) = delete;

  VulkanDevice(VulkanDevice &&o) noexcept;

  VulkanDevice &operator=(VulkanDevice &&o) = delete;

  static VulkanDevice Create(VulkanInstance &p_Instance, VkSurfaceKHR p_Surface);

  void waitIdle();

  // Getter
  [[nodiscard]] VkDevice getDevice() const { return device; }

  [[nodiscard]] VkQueue getGraphicsQueue() const { return graphicsQueue; }

  [[nodiscard]] VkQueue getPresentQueue() const { return presentQueue; }

  [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }

  [[nodiscard]] VkPhysicalDeviceProperties getProperties() const { return properties; }

  [[nodiscard]] uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }

  [[nodiscard]] uint32_t getPresentQueueFamily() const { return presentQueueFamily; }

  // Wrapper for external calls
  [[nodiscard]] bool checkDeviceExtensionSupport() const;

  [[nodiscard]] QueueFamilyIndices findQueueFamilies() const;

  [[nodiscard]] SwapChainSupportDetails querySwapChainSupport() const;

  VkFormat
  findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

private:
  VulkanInstance &instance;

  VkSurfaceKHR surface;

  VkPhysicalDevice physicalDevice;

  VkDevice device;

  uint32_t graphicsQueueFamily;
  uint32_t presentQueueFamily;

  VkQueue graphicsQueue;
  VkQueue presentQueue;

  VkPhysicalDeviceProperties properties;
};

