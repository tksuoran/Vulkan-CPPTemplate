cmake_minimum_required(VERSION 3.12)

project(Vulkan-CPPTemplate)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(executable src/main.cpp)
set_target_properties(executable PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/Vulkan-Headers)
add_subdirectory(external/SPIRV-Headers)
add_subdirectory(external/SPIRV-Tools)
add_subdirectory(external/glslang)
add_subdirectory(external/Vulkan-Loader)
add_subdirectory(external/Vulkan-ValidationLayers)
# TODO VulkanMemoryAllocator

target_include_directories(executable PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/Vulkan-Headers/include
)

target_link_libraries(executable PRIVATE
    Vulkan::Vulkan
)
