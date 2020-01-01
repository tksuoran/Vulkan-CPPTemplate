#!/usr/bin/env bash

set -e

git init
mkdir -p external

# Vulkan SDK
git submodule add git@github.com:KhronosGroup/glslang.git external/glslang
git submodule add git@github.com:KhronosGroup/SPIRV-Cross.git external/SPIRV-Cross
git submodule add git@github.com:KhronosGroup/SPIRV-Headers.git external/SPIRV-Headers
git submodule add git@github.com:KhronosGroup/SPIRV-Tools.git external/SPIRV-Tools
git submodule add git@github.com:KhronosGroup/Vulkan-Headers.git external/Vulkan-Headers
git submodule add git@github.com:KhronosGroup/Vulkan-Loader.git external/Vulkan-Loader
git submodule add git@github.com:KhronosGroup/Vulkan-ValidationLayers.git external/Vulkan-ValidationLayers
git submodule add git@github.com:KhronosGroup/Vulkan-Tools.git external/Vulkan-Tools

# Vulkan extras
git submodule add git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git external/VulkanMemoryAllocator

# C++ extras
git submodule add git@github.com:fmtlib/fmt.git external/fmt
git submodule add git@github.com:microsoft/GSL.git external/GSL

