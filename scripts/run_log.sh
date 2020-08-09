#!/bin/bash
VK_LAYER_PATH=build/subprojects/Vulkan-ValidationLayers/layers LD_LIBRARY_PATH=build/subprojects/Vulkan-ValidationLayers/layers build/executable >vk_log.txt 2>vk_log2.txt
