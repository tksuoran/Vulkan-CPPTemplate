#ifndef device_hpp_vipu_graphics
#define device_hpp_vipu_graphics

#include <memory>

#include "graphics/physical_device.hpp"
#include "graphics/vulkan.hpp"

namespace vipu
{

class Surface;

/// Owns:
///  - Vulkan device
///  - Vulkan queue
class Device
{
public:
    Device(Physical_device *physical_device,
           Surface         *surface);

    auto get()
    -> vk::Device;

    auto get_queue_family_indices()
    -> const Queue_family_indices &;

    auto get_surface_formats(vk::SurfaceKHR vk_surface)
    -> std::vector<vk::SurfaceFormatKHR>;

private:
    vk::UniqueDevice     m_vk_device;
    vk::PhysicalDevice   m_vk_physical_device;
    vk::Queue            m_vk_queue;
    Queue_family_indices m_queue_family_indices;
};

} // namespace vipu

#endif // device_hpp_vipu_graphics
