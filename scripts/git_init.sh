#!/usr/bin/env bash

set -e

git init
mkdir -p subprojects

# Vulkan SDK
git submodule add git@github.com:KhronosGroup/glslang.git                 subprojects/glslang
git submodule add git@github.com:KhronosGroup/SPIRV-Cross.git             subprojects/SPIRV-Cross
git submodule add git@github.com:KhronosGroup/SPIRV-Headers.git           subprojects/SPIRV-Headers
git submodule add git@github.com:KhronosGroup/SPIRV-Tools.git             subprojects/SPIRV-Tools
git submodule add git@github.com:KhronosGroup/Vulkan-Headers.git          subprojects/Vulkan-Headers
git submodule add git@github.com:KhronosGroup/Vulkan-Loader.git           subprojects/Vulkan-Loader
git submodule add git@github.com:KhronosGroup/Vulkan-ValidationLayers.git subprojects/Vulkan-ValidationLayers
git submodule add git@github.com:KhronosGroup/Vulkan-Tools.git            subprojects/Vulkan-Tools

# Vulkan extras
git submodule add git@github.com:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git subprojects/VulkanMemoryAllocator

# C++ extras
#git submodule add git@github.com:fmtlib/fmt.git subproecjts/fmt
#git submodule add git@github.com:microsoft/GSL.git external/GSL

