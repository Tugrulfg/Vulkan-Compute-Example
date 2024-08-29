#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>


typedef struct {
    VkBuffer srcBuffer;
    VkDeviceMemory srcMemory;
    VkDeviceSize srcBufferSize;
    VkBuffer resBuffer;
    VkDeviceMemory resMemory;
    VkDeviceSize resBufferSize;

    uint32_t value[2560];
    uint32_t res[2560];
    size_t arraySize;
    const char *computeShaderPath;
} Compute;

typedef struct {
    const char *applicationName;
    const char *engineName;

    uint32_t apiVersion;

    uint32_t queueFamily;

    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue queue;

    VkPipeline computePipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkFence fence;

    Compute compute;
} App;


#endif