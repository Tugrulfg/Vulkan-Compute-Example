#ifndef APP_H
#define APP_H

#include "example.h"
#include "utils.h"

void createInstance(App* app);

void pickPhysicalDevice(App* app);

void createLogicalDevice(App* app);

void queueFamilySelect(App* app);

void createCommandPool(App* app);

void createCommandBuffers(App* app);

void createStorageBuffer(App* app);

void createDescriptorSetLayout(App* app);

void createComputePipeline(App* app);

void createDescriptorPool(App* app);

void createDescriptorSets(App* app);

void recordCommandBuffer(App* app);

void retrieveData(App* app);

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice device, VkPhysicalDevice pDevice, VkBuffer* buffer, VkDeviceMemory* bufferMemory);

uint32_t findMemoryType(VkPhysicalDevice pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue);

// Cleans up the Vulkan resources
void cleanup(App *app);

#endif