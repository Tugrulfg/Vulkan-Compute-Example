#include "../include/example.h"
#include "../include/app.h"

int main() {
    App app = {
            .applicationName = "Vulkan Example",
            .engineName = "",
            .apiVersion = VK_API_VERSION_1_3,
            .compute = {
                .computeShaderPath = "../res/shaders/SimpleExample.comp.spv",
                .arraySize = 256
            }
    };

    for(int i = 0; i < 256; i++){
        app.compute.value1[i] = (float)i;
        app.compute.value2[i] = (float)i;
    }


    printf("Create instance\n");
    createInstance(&app);
    printf("Pick physical device\n");
    pickPhysicalDevice(&app);
    printf("Create logical device\n");
    createLogicalDevice(&app);
    printf("Create command pool\n");
    createCommandPool(&app);
    printf("Create command buffers\n");
    createCommandBuffers(&app);
    printf("Create storage buffer\n");
    createStorageBuffer(&app);
    printf("Create descriptor set layout\n");
    createDescriptorSetLayout(&app);
    printf("Create compute pipeline\n");
    createComputePipeline(&app);
    printf("Create descriptor pool\n");
    createDescriptorPool(&app);
    printf("Create descriptor sets\n");
    createDescriptorSets(&app);
    printf("Record command buffer\n");
    recordCommandBuffer(&app);
    printf("Retrieve data\n");
    retrieveData(&app);

    printf("Result:\n");
    for(int i = 0; i < 256; i++) 
        printf("%f\n", app.compute.res[i]);

    // createSyncObjects(&app);


    cleanup(&app);
    return EXIT_SUCCESS;
}
