#include <gsl/gsl>

#include "log/log.hpp"
#include "graphics/log.hpp"
#include "graphics/configuration.hpp"
#include "graphics/display_surface.hpp"
#include "graphics/instance.hpp"
#include "graphics/physical_device.hpp"

namespace vipu
{

Display_surface::Display_surface(Configuration   *configuration,
                                 Instance        *instance,
                                 Physical_device *physical_device)
    : Surface{configuration, instance, physical_device}
{
    Expects(configuration->surface_type == Surface::Type::eDisplay);

    m_display = physical_device->choose_display();

    VERIFY(m_display != nullptr);

    auto m_vk_display = m_display->get();

    // TODO Choose display mode
    m_display_mode = m_display->get_mode_properties().front().displayModeProperties.displayMode;

    m_display_plane_index = m_display->get_plane_index();

    log_vulkan.info("Chose display {}\n", m_display_index);
    log_vulkan.info("Chose display plane {}\n", m_display_plane_index);

    Ensures(m_display_plane_index != std::numeric_limits<uint32_t>::max());

    vk::DisplayPlaneInfo2KHR plane_info_key{m_display_mode, m_display_plane_index};
    auto capabilities = m_vk_physical_device.getDisplayPlaneCapabilities2KHR(plane_info_key);

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

    m_vk_surface = m_vk_instance.createDisplayPlaneSurfaceKHRUnique(surface_create_info);

    log_vulkan.trace("Created surface {} x {} on plane index {}\n",
                     extent.width,
                     extent.height,
                     m_display_plane_index);

    Ensures(m_vk_surface);
}

} // namespace vipu
