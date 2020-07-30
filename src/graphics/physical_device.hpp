#ifndef physical_device_hpp_vipu_graphics
#define physical_device_hpp_vipu_graphics

#include <cstdint>
#include <limits>
#include <vector>

#include "graphics/display.hpp"
#include "graphics/vulkan.hpp"

namespace vipu
{

class Configuration;
class Display;
class Instance;
class Surface;

struct Queue_family_indices
{
    uint32_t graphics{std::numeric_limits<uint32_t>::max()};
    uint32_t present {std::numeric_limits<uint32_t>::max()};
};

class Physical_device
{
public:
    Physical_device() = default;

    Physical_device(Configuration      *configuration,
                    Instance           *instance,
                    vk::PhysicalDevice  physical_device);

    // Only to be used with Display WSI
    void scan_displays();

    // Only to be used with Display WSI
    auto choose_display()
    -> Display *;

    auto choose_queue_family_indices(Surface *surface)
    -> Queue_family_indices;

    auto get()
    -> vk::PhysicalDevice;

private:
    Configuration                          *m_configuration{nullptr};
    Instance                               *m_instance     {nullptr};
    vk::Instance                            m_vk_instance;
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
