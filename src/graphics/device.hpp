#ifndef device_hpp_vipu_graphics
#define device_hpp_vipu_graphics

#include <memory>

#include "graphics/physical_device.hpp"
#include "graphics/vulkan.hpp"

namespace vipu
{

class Context;

class Device
{
public:
    Device(Context &context);

    auto get()
    -> vk::Device;

    auto get_queue_family_indices()
    -> const Queue_family_indices &;

    auto get_queue()
    -> vk::Queue;

private:
    vk::UniqueDevice     m_vk_device;
    vk::Queue            m_vk_queue;
    Queue_family_indices m_queue_family_indices;
};

} // namespace vipu

#endif // device_hpp_vipu_graphics
