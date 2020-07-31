#include <gsl/gsl>

#include "graphics/display.hpp"
#include "graphics/log.hpp"
#include "graphics/physical_device.hpp"

namespace vipu
{

Display::Display(vk::PhysicalDevice         vk_physical_device,
                 uint32_t                   display_index,
                 vk::DisplayProperties2KHR &properties)
:   m_properties   {properties}
,   m_display_index{display_index}
{
    Expects(vk_physical_device);
    Expects(m_vk_display);

    std::string name{m_properties.displayProperties.displayName};
    log_vulkan.trace("Display {}: {}\n", display_index, name);
    m_mode_properties = vk_physical_device.getDisplayModeProperties2KHR(m_properties.displayProperties.display);

    log_vulkan.trace("\tModes: {}\n", m_mode_properties.size());
    for (size_t mode_index = 0;
        mode_index < m_mode_properties.size();
        ++mode_index)
    {
        auto &p = m_mode_properties[mode_index].displayModeProperties.parameters;
        log_vulkan.trace("\t\tMode {}: {} x {} @ {} Hz\n",
                         mode_index,
                         p.visibleRegion.width,
                         p.visibleRegion.height,
                         p.refreshRate);
    }

    auto plane_properties = vk_physical_device.getDisplayPlaneProperties2KHR();
    log_vulkan.trace("\tPlanes: {}\n", plane_properties.size());
    m_planes.resize(plane_properties.size());
    m_display_plane_index = std::numeric_limits<uint32_t>::max();
    for (uint32_t plane_index = 0;
        plane_index < plane_properties.size();
        ++plane_index)
    {
        m_planes[plane_index] = Plane(vk_physical_device,
                                      m_vk_display,
                                      plane_index,
                                      plane_properties[plane_index]);
        auto &plane = m_planes[plane_index];
        if (plane.current && plane.supported)
        {
            m_display_plane_index = plane_index;
        }
    }

    VERIFY(m_display_plane_index != std::numeric_limits<uint32_t>::max());
    log_vulkan.info("Chose display plane {}\n", m_display_plane_index);
}

auto Display::get()
-> vk::DisplayKHR
{
    return m_vk_display;
}

auto Display::get_mode_properties()
-> const std::vector<vk::DisplayModeProperties2KHR> &
{
    return m_mode_properties;
}

auto Display::get_plane_index()
-> uint32_t
{
    return m_display_plane_index;
}

Plane::Plane(vk::PhysicalDevice              vk_physical_device,
             vk::DisplayKHR                  vk_display,
             uint32_t                        plane_index,
             vk::DisplayPlaneProperties2KHR &properties)
:   properties{properties}
{
    supported_displays = vk_physical_device.getDisplayPlaneSupportedDisplaysKHR(plane_index);
    current = properties.displayPlaneProperties.currentDisplay == vk_display;
    supported = std::find(supported_displays.begin(),
                          supported_displays.end(),
                          vk_display) != supported_displays.end();
    log_vulkan.trace("\t\tPlane {}: current display: {}, display supported: {}, has display: {}, current stack index: {}, supported displays: {}\n",
                     plane_index,
                     current   ? "yes" : "no",
                     supported ? "yes" : "no",
                     static_cast<bool>(properties.displayPlaneProperties.currentDisplay) ? "yes" : "no",
                     properties.displayPlaneProperties.currentStackIndex,
                     supported_displays.size());
}

} // namespace vipu
