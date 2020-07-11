cmake_minimum_required(VERSION 3.12)

project(Vulkan-CPPTemplate)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(executable
    src/main.cpp
    src/log.cpp
    src/log.hpp
)

set_target_properties(executable PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

add_subdirectory(subprojects/fmt)
add_subdirectory(subprojects/GSL)
add_subdirectory(subprojects/Vulkan-Headers)
add_subdirectory(subprojects/SPIRV-Headers)
add_subdirectory(subprojects/SPIRV-Tools)
add_subdirectory(subprojects/glslang)
add_subdirectory(subprojects/Vulkan-Loader)
add_subdirectory(subprojects/Vulkan-ValidationLayers)
# add_subdirectory(subprojects/VulkanMemoryAllocator)

target_include_directories(executable PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(executable PRIVATE
    fmt::fmt
    Microsoft.GSL::GSL
    Vulkan::Vulkan
)
