#include "../include/app.h"

void createInstance(App* app){
    uint32_t requiredExtensionsCount = 0;
    const char *requiredExtensions[] = {

    };

    EXPECT(vkCreateInstance(&(VkInstanceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &(VkApplicationInfo) {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .pApplicationName = app->applicationName,
                    .pEngineName = app->engineName,
                    .apiVersion = app->apiVersion,
            },
            .enabledExtensionCount = requiredExtensionsCount,
            .ppEnabledExtensionNames = requiredExtensions,
    }, NULL, &app->instance), "Couldn't create instance")
}

void pickPhysicalDevice(App* app){
    uint32_t count;
    EXPECT(vkEnumeratePhysicalDevices(app->instance, &count, NULL), "Couldn't enumerate physical devices count")
    EXPECT(count == 0, "Couldn't find a vulkan supported physical device")
    VkResult result = vkEnumeratePhysicalDevices(app->instance, &(uint32_t){1}, &app->physicalDevice);
    if(result != VK_INCOMPLETE) EXPECT(result, "Couldn't enumerate physical devices")
}

void createLogicalDevice(App* app){
    queueFamilySelect(app);

    EXPECT(vkCreateDevice(app->physicalDevice, &(VkDeviceCreateInfo) {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pQueueCreateInfos = &(VkDeviceQueueCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .queueFamilyIndex = app->queueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &(float){1.0},
            },
            .queueCreateInfoCount = 1,
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = &(const char *) {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
    }, NULL, &app->device), "Couldn't create device and queues")

    vkGetDeviceQueue(app->device, app->queueFamily, 0, &app->queue);
}

void queueFamilySelect(App* app){
    app->queueFamily = UINT32_MAX;

    // Get queue family properties of the physical device
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(app->physicalDevice, &queueFamilyCount, NULL);
	EXPECT(queueFamilyCount == 0, "Couldn't get the family queue count")
	VkQueueFamilyProperties *queueFamilyProperties = malloc(queueFamilyCount*sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(app->physicalDevice, &queueFamilyCount, queueFamilyProperties);

    for (uint32_t i=0; i<queueFamilyCount; i++){
		if(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT){
			app->queueFamily = i;
            free(queueFamilyProperties);
            return;
		}
	}
    EXPECT(app->queueFamily == UINT32_MAX, "Couldn't find a suitable queue family")
    free(queueFamilyProperties);
}

void createCommandPool(App* app){
    EXPECT(vkCreateCommandPool(app->device, &(VkCommandPoolCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = app->queueFamily,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    }, NULL, &app->commandPool), "Couldn't create command pool")
}

void createCommandBuffers(App* app){
    EXPECT(vkAllocateCommandBuffers(app->device, &(VkCommandBufferAllocateInfo) {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = app->commandPool,
        .commandBufferCount = (uint32_t)1,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    }, &app->commandBuffer), "Couldn't allocate command buffer")
}

void createStorageBuffer(App* app){

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(app->compute.srcBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, app->device, app->physicalDevice, &stagingBuffer, &stagingBufferMemory);

    void* data;
    vkMapMemory(app->device, stagingBufferMemory, 0, app->compute.srcBufferSize, 0, &data);
    memcpy(data, app->compute.value, (size_t) app->compute.srcBufferSize);
    vkUnmapMemory(app->device, stagingBufferMemory);

    createBuffer(app->compute.srcBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, app->device, app->physicalDevice, &app->compute.srcBuffer, &app->compute.srcMemory);

    copyBuffer(stagingBuffer, app->compute.srcBuffer, app->compute.srcBufferSize, app->device, app->commandPool, app->queue);

    // Create the result buffer
    createBuffer(app->compute.resBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, app->device, app->physicalDevice, &app->compute.resBuffer, &app->compute.resMemory);

    vkDestroyBuffer(app->device, stagingBuffer, NULL);
    vkFreeMemory(app->device, stagingBufferMemory, NULL);
}

void createDescriptorSetLayout(App* app){
    VkDescriptorSetLayoutBinding layoutBindings[3] = {
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        },
        {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        }
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 2,
        .pBindings = layoutBindings,
    };

    EXPECT(vkCreateDescriptorSetLayout(app->device, &layoutInfo, NULL, &app->descriptorSetLayout), "Couldn't create descriptor set layout")
}

void createComputePipeline(App* app){
    const char *shaderEntryFunctionName = "main";

    VkShaderModule computeShaderModule = load_shader(app->compute.computeShaderPath, app);
    
    VkPipelineShaderStageCreateInfo shaderStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .module = computeShaderModule,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .pName = shaderEntryFunctionName,
    };

    EXPECT(vkCreatePipelineLayout(app->device, &(VkPipelineLayoutCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &app->descriptorSetLayout,
    }, NULL, &app->pipelineLayout), "Couldn't create pipeline layout")

    EXPECT(vkCreateComputePipelines(app->device, NULL, 1, &(VkComputePipelineCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .layout = app->pipelineLayout,
        .stage = shaderStage,
    }, NULL, &app->computePipeline), "Couldn't create graphics pipeline")

    vkDestroyPipelineLayout(app->device, app->pipelineLayout, NULL);
    vkDestroyShaderModule(app->device, computeShaderModule, NULL);
}

void createDescriptorPool(App* app){
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = (uint32_t)2,
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = (uint32_t)1,
    };

    EXPECT(vkCreateDescriptorPool(app->device, &poolInfo, NULL, &app->descriptorPool), "Couldn't create descriptor pool")
}

void createDescriptorSets(App* app){
    
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = app->descriptorPool,
        .descriptorSetCount = (uint32_t)1,
        .pSetLayouts = &app->descriptorSetLayout,
    };
    
    EXPECT(vkAllocateDescriptorSets(app->device, &allocInfo, &app->descriptorSet), "Couldn't allocate descriptor sets")

        VkDescriptorBufferInfo bufferInfoSrc = {
            .buffer = app->compute.srcBuffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        VkDescriptorBufferInfo bufferInfoRes = {
            .buffer = app->compute.resBuffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
        };

        VkWriteDescriptorSet descriptorWrites[2] = {
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = app->descriptorSet,
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &bufferInfoSrc,
            },
            {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = app->descriptorSet,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .pBufferInfo = &bufferInfoRes,
            }
        };

        vkUpdateDescriptorSets(app->device, 2, descriptorWrites, 0, NULL);
}

void createSyncObjects(App* app){
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    };
    vkCreateFence(app->device, &fenceInfo, NULL, &app->fence);
}

void recordCommandBuffer(App* app){
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(app->commandBuffer, &beginInfo);

    vkCmdBindPipeline(app->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, app->computePipeline);
    vkCmdBindDescriptorSets(app->commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, app->pipelineLayout, 0, 1, &app->descriptorSet, 0, NULL);

    vkCmdDispatch(app->commandBuffer, app->compute.arraySize / 1024, 1, 1);

    vkEndCommandBuffer(app->commandBuffer);
}

void submitCommandBuffer(App* app){
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &app->commandBuffer,

    };

    vkQueueSubmit(app->queue, 1, &submitInfo, app->fence);
}

void retrieveData(App* app){
    void* data;

    EXPECT(vkMapMemory(app->device, app->compute.resMemory, 0, app->compute.resBufferSize, 0, &data), "Couldn't map memory")
    vkWaitForFences(app->device, 1, &app->fence, VK_TRUE, UINT64_MAX);

    for(int i = 0; i < app->compute.resBufferSize / (uint32_t)sizeof(uint32_t); i++)
        app->compute.res[i] = ((uint32_t*)data)[i];

    // Process the data
    vkUnmapMemory(app->device, app->compute.resMemory);
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice device, VkPhysicalDevice pDevice, VkBuffer* buffer, VkDeviceMemory* bufferMemory){
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    EXPECT(vkCreateBuffer(device, &bufferInfo, NULL, buffer), "Couldn't create buffer")

    VkMemoryRequirements memRequirements = {};
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(pDevice, memRequirements.memoryTypeBits, properties),
    };

    EXPECT(vkAllocateMemory(device, &allocInfo, NULL, bufferMemory), "Couldn't allocate buffer memory")

    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

uint32_t findMemoryType(VkPhysicalDevice pDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(pDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    EXPECT(1, "Couldn't find suitable memory type")
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue){
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1,
    };
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
    };
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void cleanup(App *app) {

    vkDestroyBuffer(app->device, app->compute.srcBuffer, NULL);
    vkFreeMemory(app->device, app->compute.srcMemory, NULL);
    vkDestroyBuffer(app->device, app->compute.resBuffer, NULL);
    vkFreeMemory(app->device, app->compute.resMemory, NULL);

    // Destroy graphics pipeline
    vkDestroyPipeline(app->device, app->computePipeline, NULL);

    vkDestroyDescriptorPool(app->device, app->descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(app->device, app->descriptorSetLayout, NULL);

    // Destroy command pool
    vkFreeCommandBuffers(app->device, app->commandPool, 1, &app->commandBuffer);
    vkDestroyCommandPool(app->device, app->commandPool, NULL);

    vkDestroyFence(app->device, app->fence, NULL);

    // Destroy logical device
    vkDestroyDevice(app->device, NULL);

    // Destroy instance
    vkDestroyInstance(app->instance, NULL);
}
