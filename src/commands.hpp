#pragma once

#include <vulkan/vulkan.hpp>

#include <vector>

#include "queueFamilyIndices.hpp"

class Commands {
public:
    VkCommandBuffer beginSingleTime(VkQueue graphicsQueue, VkDevice device);
    void endSingleTime(VkCommandBuffer commandBuffer, VkQueue graphicsQueue, VkDevice device);

    void createPool(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface);

    void createBuffers(VkDevice device, size_t maxFramesInFlight);
    void resetBuffers(const uint32_t imageIndex, const uint32_t currentFrame);
    VkCommandBuffer getBuffer(const uint32_t currentFrame);

    void destroy(VkDevice device);

private:
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> buffers;
};