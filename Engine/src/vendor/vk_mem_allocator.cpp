#include "platform.h"
#include <cstdio>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1

PUSH_MINIMAL_WARNING_LEVEL

#include "vk_mem_alloc.h"

POP_MINIMAL_WARNING_LEVEL

// See https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/quick_start.html