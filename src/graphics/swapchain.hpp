#ifndef swapchain_hpp_vipu_graphics
#define swapchain_hpp_vipu_graphics

#include "graphics/vulkan.hpp"

namespace vipu
{

class Device;
class Surface;

class Swapchain
{
public:
    Swapchain(Device  *device,
              Surface *surface);

    auto get()
    -> vk::SwapchainKHR;

protected:
    vk::UniqueSwapchainKHR m_vk_swapchain;
};

} // namespace vipu

#endif // surface_hpp_vipu_graphics
