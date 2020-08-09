#ifndef swapchain_hpp_vipu_graphics
#define swapchain_hpp_vipu_graphics

#include "graphics/vulkan.hpp"

namespace vipu
{

class Context;

class Swapchain
{
public:
    Swapchain(Context &context);

    auto get()
    -> vk::SwapchainKHR;

    auto get_surface_format()
    -> vk::SurfaceFormatKHR;

protected:
    vk::UniqueSwapchainKHR m_vk_swapchain;
    vk::SurfaceFormatKHR   m_surface_format;
};

} // namespace vipu

#endif // surface_hpp_vipu_graphics
