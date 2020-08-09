#include <gsl/gsl>

#include "log/log.hpp"
#include "graphics/context.hpp"
#include "graphics/display_surface.hpp"
#include "graphics/instance.hpp"
#include "graphics/log.hpp"
#include "graphics/physical_device.hpp"

namespace vipu
{

Display_surface::Display_surface(Context &context)
{
    Expects(context.physical_device != nullptr);
    Expects(context.vk_physical_device);

    context.display = context.physical_device->choose_display(true);
    VERIFY(context.display != nullptr);

    context.vk_display = context.display->get();
    VERIFY(context.vk_display);

    // TODO Choose display mode
    m_display_mode = context.display->get_mode_properties().front().displayModeProperties.displayMode;

    m_display_plane_index = context.display->get_plane_index();

    auto &properties = context.display->get_properties();
    std::string name{properties.displayProperties.displayName};
    log_vulkan.info("Chose display {}\n", name);
    log_vulkan.info("Chose display plane {}\n", m_display_plane_index);

    Ensures(m_display_plane_index != std::numeric_limits<uint32_t>::max());

    vk::DisplayPlaneInfo2KHR plane_info_key{m_display_mode, m_display_plane_index};
    auto capabilities = context.vk_physical_device.getDisplayPlaneCapabilities2KHR(plane_info_key);

    auto extent = capabilities.capabilities.maxDstExtent;

    VERIFY(extent.width > 0);
    VERIFY(extent.height > 0);

    uint32_t plane_stack_index{0};
    float plane_global_alpha{1.0f};
    vk::DisplaySurfaceCreateInfoKHR surface_create_info {
        vk::DisplaySurfaceCreateFlagsKHR(),
        m_display_mode,
        m_display_plane_index,
        plane_stack_index,
        vk::SurfaceTransformFlagBitsKHR::eIdentity,
        plane_global_alpha,
        vk::DisplayPlaneAlphaFlagBitsKHR::eOpaque,
        extent
    };

    m_vk_surface = context.vk_instance.createDisplayPlaneSurfaceKHRUnique(surface_create_info);

    log_vulkan.trace("Created surface {} x {} on plane index {}\n",
                     extent.width,
                     extent.height,
                     m_display_plane_index);

    Surface::get_properties(context);

    context.vk_surface = m_vk_surface.get();

    Ensures(m_vk_surface);
    Ensures(context.vk_surface);
}

} // namespace vipu
