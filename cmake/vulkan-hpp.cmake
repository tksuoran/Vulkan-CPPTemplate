# Copyright 2019 The VKB Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

message(STATUS "VKB Vulkan-Hpp: ${VKB_VULKAN_HPP_SOURCE_DIR}")

set(VulkanRegistry_DIR "${VKB_VULKAN_DOCS_SOURCE_DIR}")
set(VULKAN_HPP_VULKAN_HEADERS_SRC_DIR "${VKB_VULKAN_HEADERS_SOURCE_DIR}")
set(VULKAN_HPP_PATH "${CMAKE_CURRENT_BINARY_DIR}/third_party/vulkan-hpp")
set(VULKAN_HPP_TINYXML2_SRC_DIR "${VKB_TINYXML2_SOURCE_DIR}")
set(VULKAN_HPP_INSTALL ON CACHE BOOL "vulkan-hpp install")

if (ANDROID)
  set(VULKAN_HPP_RUN_GENERATOR OFF CACHE BOOL "vulkan-hpp disable generator")
  message(STATUS "VKB Vulkan-Hpp: Disable generator on Android")
else()
  set(VULKAN_HPP_RUN_GENERATOR ON CACHE BOOL "vulkan-hpp run generator")
endif()

# TODO(dsinclair) This uses FindPackage(Vulkan) which may use the cmake version
# which will fail if VULKAN_SDK is not set.
set(TESTS_BUILD OFF CACHE BOOL "hpp tests off")
message(STATUS "VKB Vulkan-Hpp: Disable tests")

set(SAMPLES_BUILD OFF CACHE BOOL "hpp samples off")

add_subdirectory(${VKB_VULKAN_HPP_SOURCE_DIR})
