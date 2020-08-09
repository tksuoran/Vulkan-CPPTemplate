#include <gsl/gsl>

#include "graphics/surface.hpp"
#include "graphics/context.hpp"
#include "graphics/instance.hpp"
#include "graphics/log.hpp"

namespace vipu
{

void Surface::get_properties(Context &context)
{
    auto vk_surface = m_vk_surface.get();
    VERIFY(vk_surface);

    log_vulkan.trace("Surface properties:\n");

    std::vector<vk::PresentModeKHR> present_modes = context.vk_physical_device.getSurfacePresentModesKHR(vk_surface);
    for (auto present_mode : present_modes)
    {
        log_vulkan.trace("    present mode : {}\n", vk::to_string(present_mode));
    }

    m_surface_capabilities = context.vk_physical_device.getSurfaceCapabilitiesKHR(vk_surface);
    log_vulkan.trace("    minImageCount           : {}\n",      m_surface_capabilities.minImageCount);
    log_vulkan.trace("    maxImageCount           : {}\n",      m_surface_capabilities.maxImageCount);
    log_vulkan.trace("    currentExtent           : {} x {}\n", m_surface_capabilities.currentExtent.width, m_surface_capabilities.currentExtent.height);
    log_vulkan.trace("    minExtent               : {} x {}\n", m_surface_capabilities.minImageExtent.width, m_surface_capabilities.minImageExtent.height);
    log_vulkan.trace("    maxExtent               : {} x {}\n", m_surface_capabilities.maxImageExtent.width, m_surface_capabilities.maxImageExtent.height);
    log_vulkan.trace("    maxImageArrayLayers     : {}\n",      m_surface_capabilities.maxImageArrayLayers);
    log_vulkan.trace("    supportedTransforms     : {}\n",      vk::to_string(m_surface_capabilities.supportedTransforms));
    log_vulkan.trace("    currentTransform        : {}\n",      vk::to_string(m_surface_capabilities.currentTransform));
    log_vulkan.trace("    supportedCompositeAlpha : {}\n",      vk::to_string(m_surface_capabilities.supportedCompositeAlpha));
    log_vulkan.trace("    supportedUsageFlags     : {}\n",      vk::to_string(m_surface_capabilities.supportedUsageFlags));
}

auto Surface::get()
-> vk::SurfaceKHR
{
    return m_vk_surface.get();
}

auto Surface::get_present_modes()
-> const std::vector<vk::PresentModeKHR> &
{
    return m_present_modes;
}

auto Surface::get_capabilities()
-> const vk::SurfaceCapabilitiesKHR &
{
    return m_surface_capabilities;
}


} // namespace vipu
