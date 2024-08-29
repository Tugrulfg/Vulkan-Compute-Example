cmake_minimum_required(VERSION 3.23)
project(VulkanComputeExample C)

set(CMAKE_C_STANDARD 17)

add_executable(VulkanComputeExample src/main.c src/utils.c src/app.c)
target_link_libraries(VulkanComputeExample vulkan m)

target_include_directories(VulkanComputeExample PRIVATE include)