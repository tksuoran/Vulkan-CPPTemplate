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
///  - Vulkan surface
class Device
{
public:
    Device(Physical_device *physical_device,
           Surface         *surface);

    void create_swapchain();

private:
    Physical_device        *m_physical_device;
    vk::PhysicalDevice       m_vk_physical_device;
    vk::UniqueDevice         m_vk_device;
    vk::Queue                m_vk_queue;
    vk::UniqueSwapchainKHR   m_vk_swapchain;
    Surface                 *m_surface;
    Queue_family_indices     m_queue_family_indices;
};

} // namespace vipu

#endif // device_hpp_vipu_graphics
