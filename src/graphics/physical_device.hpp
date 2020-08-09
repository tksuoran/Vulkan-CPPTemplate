#ifndef physical_device_hpp_vipu_graphics
#define physical_device_hpp_vipu_graphics

#include <cstdint>
#include <limits>
#include <vector>

#include "graphics/display.hpp"
#include "graphics/vulkan.hpp"

namespace vipu
{

class Context;
class Display;

struct Queue_family_indices
{
    uint32_t graphics{std::numeric_limits<uint32_t>::max()};
    uint32_t present {std::numeric_limits<uint32_t>::max()};
};

class Physical_device
{
public:
    Physical_device() = default;

    Physical_device(Context &context, vk::PhysicalDevice vk_physical_device);

    // Only to be used with Display WSI
    void scan_displays(Context &context);

    // Only to be used with Display WSI
    auto choose_display(bool use_current_display)
    -> Display *;

    auto choose_queue_family_indices(Context &context)
    -> Queue_family_indices;

    auto get()
    -> vk::PhysicalDevice;

private:
    vk::PhysicalDevice                      m_vk_physical_device;
    std::vector<vk::ExtensionProperties>    m_extensions;
    vk::PhysicalDeviceProperties2           m_properties;
    std::vector<vk::QueueFamilyProperties2> m_queue_family_properties;
    vk::PhysicalDeviceFeatures2             m_features;
    vk::PhysicalDeviceMemoryProperties2     m_memory_properties;
    std::vector<Display>                    m_displays;
};

}

#endif // physical_device_hpp_vipu_graphics
