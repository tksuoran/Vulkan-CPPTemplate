#ifndef context_hpp_vipu_graphics
#define context_hpp_vipu_graphics

#include <limits>
#include <cstdint>
#include "graphics/vulkan.hpp"
#include "graphics/surface.hpp"

namespace vipu
{

class Device;
class Display;
class Instance;
class Physical_device;
class Surface;
class Swapchain;

struct Context
{
    vk::Device         vk_device;
    vk::Queue          vk_queue;
    vk::PhysicalDevice vk_physical_device;
    vk::Instance       vk_instance;
    vk::SurfaceKHR     vk_surface;
    vk::DisplayKHR     vk_display;
    vk::SwapchainKHR   vk_swapchain;

    Device            *device               {nullptr};
    Display           *display              {nullptr};
    Instance          *instance             {nullptr};
    Physical_device   *physical_device      {nullptr};
    Surface           *surface              {nullptr};
    Swapchain         *swapchain            {nullptr};
    uint32_t           graphics_queue_family_index{std::numeric_limits<uint32_t>::max()};
    uint32_t           present_queue_family_index {std::numeric_limits<uint32_t>::max()};

    uint64_t           frame_number{0};
    bool               quit        {false};
    bool               pause       {false};

    Surface::Type      surface_type{Surface::Type::eNone};
};

} // namespace vipu

#endif // context_hpp_vipu_graphics
