#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>


typedef struct {
    VkBuffer buffer1;
    VkDeviceMemory memory1;
    VkDeviceSize bufferSize1;
    VkBuffer buffer2;
    VkDeviceMemory memory2;
    VkDeviceSize bufferSize2;
    VkBuffer resBuffer;
    VkDeviceMemory resMemory;
    VkDeviceSize resBufferSize;

    float value1[256];
    float value2[256];
    float res[256];
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

    Compute compute;
} App;


#endif