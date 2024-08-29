#include "../include/example.h"
#include "../include/app.h"
#include "time.h"

uint32_t fibonacci(uint32_t n);

int main() {
    App app = {
            .applicationName = "Vulkan Compute Example",
            .engineName = "",
            .apiVersion = VK_API_VERSION_1_3,
            .compute = {
                .computeShaderPath = "../res/shaders/Fibonacci.comp.spv",
                .arraySize = 2560,
                .srcBufferSize = 2560 * sizeof(uint32_t),
                .resBufferSize = 2560 * sizeof(uint32_t),
            }
    };

    // Fill the input buffer
    for(uint32_t i = 0; i < 2560; i++)
        app.compute.value[i] = i+1;

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
    printf("Create sync objects\n");
    createSyncObjects(&app);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("Record command buffer\n");
    recordCommandBuffer(&app);
    printf("Submit command buffer\n");
    submitCommandBuffer(&app);
    printf("Retrieve data\n");
    retrieveData(&app);

    clock_gettime(CLOCK_MONOTONIC, &end);

    printf("\nResult:\n");
    // Print the first 10 elements of the result
    for(int i = 0; i < 10; i++) 
        printf("%u\n", app.compute.res[i]);
    printf("\n");

    double time = (double)(end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec) / 1000000000;
    printf("GPU Time: %f\n", time);


    struct timespec start1, end1;
    uint32_t val[2560];
    uint32_t res[2560];

    for(int i = 0; i < 2560; i++)
        val[i] = (uint32_t)i+1;

    clock_gettime(CLOCK_MONOTONIC, &start1);
    for(int i = 0; i < 2560; i++)
        res[i] = fibonacci(val[i]);

    clock_gettime(CLOCK_MONOTONIC, &end1);

    double time1 = (double)(end1.tv_sec - start1.tv_sec) + (double)(end1.tv_nsec - start1.tv_nsec) / 1000000000;
    printf("CPU Time: %f\n", time1);

    cleanup(&app);
    return EXIT_SUCCESS;
}

uint32_t fibonacci(uint32_t n) {
	if(n <= 1){
		return n;
	}
	uint32_t curr = 1;
	uint32_t prev = 1;
	for(uint32_t i = 2; i < n; ++i) {
		uint32_t temp = curr;
		curr += prev;
		prev = temp;
	}
	return curr;
}