#include <gsl/gsl>

#include "graphics/physical_device.hpp"
#include "graphics/context.hpp"
#include "graphics/log.hpp"
#include "graphics/instance.hpp"
#include "graphics/surface.hpp"

namespace vipu
{

Physical_device::Physical_device(Context &context, vk::PhysicalDevice vk_physical_device)
:   m_vk_physical_device{vk_physical_device}
{
    Expects(context.instance != nullptr);
    Expects(context.vk_instance);
    Expects(vk_physical_device);

    m_extensions = m_vk_physical_device.enumerateDeviceExtensionProperties();
    log_vulkan.trace("\tFound {} device extensions\n", m_extensions.size());
    for (auto &extension : m_extensions)
    {
        std::string extension_name{extension.extensionName.data()};
        log_vulkan.trace("\tFound extension {}\n", extension_name);
    }

    m_properties = m_vk_physical_device.getProperties2();
    {
        auto &p = m_properties.properties;
        log_vulkan.trace("Device name = {}, type = {}, vendorID = {:x}, deviceID = {:x}, apiVersion = {}.{}.{}, driverVersion = {}.{}.{}\n",
                         std::string(p.deviceName),
                         vk::to_string(p.deviceType),
                         p.vendorID,
                         p.deviceID,
                         VK_VERSION_MAJOR(p.apiVersion),
                         VK_VERSION_MINOR(p.apiVersion),
                         VK_VERSION_PATCH(p.apiVersion),
                         VK_VERSION_MAJOR(p.driverVersion),
                         VK_VERSION_MINOR(p.driverVersion),
                         VK_VERSION_PATCH(p.driverVersion));
    }

    {
        vk::StructureChain<vk::PhysicalDeviceProperties2,
                           vk::PhysicalDeviceDriverProperties
        > properties = m_vk_physical_device.getProperties2<vk::PhysicalDeviceProperties2,
                                                           vk::PhysicalDeviceDriverProperties>();
        auto &p = properties.get<vk::PhysicalDeviceDriverProperties>();
        log_vulkan.trace("Driver: id = {}, name = {}, info = {}, conformanceVersion = {}.{}.{}.{}\n",
                         vk::to_string(p.driverID),
                         std::string(p.driverName),
                         std::string(p.driverInfo),
                         p.conformanceVersion.major,
                         p.conformanceVersion.minor,
                         p.conformanceVersion.subminor,
                         p.conformanceVersion.patch);
    }

    m_queue_family_properties = m_vk_physical_device.getQueueFamilyProperties2();
    m_features                = m_vk_physical_device.getFeatures2();
    m_memory_properties       = m_vk_physical_device.getMemoryProperties2();

    if (context.surface_type == Surface::Type::eDisplay)
    {
        // Scan displays connected to physical device
        scan_displays(context);
    }
}

auto Physical_device::get()
-> vk::PhysicalDevice
{
    return m_vk_physical_device;
}

void Physical_device::scan_displays(Context &context)
{
    Expects(m_vk_physical_device);

    auto display_properties = m_vk_physical_device.getDisplayProperties2KHR();
    log_vulkan.trace("Found {} displays\n", display_properties.size());
    m_displays.reserve(display_properties.size());
    for (size_t display_index = 0;
        display_index < display_properties.size();
        ++display_index)
    {
        m_displays.emplace_back(get(),
                                display_index,
                                display_properties[display_index]);
    }
    log_vulkan.trace("Parsed {} displays\n", m_displays.size());
}

auto Physical_device::choose_queue_family_indices(Context &context)
-> Queue_family_indices
{
    Expects(m_vk_physical_device);
    Expects(context.vk_surface);
    Expects(!m_queue_family_properties.empty());

    // Find queue family supporting both graphics and present
    uint32_t graphics_queue_family_index = std::numeric_limits<uint32_t>::max();
    uint32_t present_queue_family_index  = std::numeric_limits<uint32_t>::max();
    for (uint32_t queue_family_index = 0;
         queue_family_index < m_queue_family_properties.size();
         ++queue_family_index)
    {
        bool present_supported = m_vk_physical_device.getSurfaceSupportKHR(queue_family_index, context.vk_surface);
        vk::QueueFlags flags = m_queue_family_properties[queue_family_index].queueFamilyProperties.queueFlags;
        bool graphics_supported = (flags & vk::QueueFlagBits::eGraphics) == vk::QueueFlagBits::eGraphics;
        if (graphics_supported)
        {
            graphics_queue_family_index = queue_family_index;
        }
        if (present_supported)
        {
            present_queue_family_index = queue_family_index;
        }
        if (present_supported && graphics_supported)
        {
            break;
        }
    }

    VERIFY(graphics_queue_family_index != std::numeric_limits<uint32_t>::max());
    VERIFY(present_queue_family_index  != std::numeric_limits<uint32_t>::max());
    VERIFY(graphics_queue_family_index == present_queue_family_index); // TODO

    log_vulkan.info("Chose graphics queue family {}\n", graphics_queue_family_index);
    log_vulkan.info("Chose present queue family {}\n", present_queue_family_index);

    return { graphics_queue_family_index, present_queue_family_index};
}

auto Physical_device::choose_display(bool use_current_display)
-> Display *
{
    Expects(!m_displays.empty());

    // TODO Choose display
    for (auto &display : m_displays)
    {
        if (use_current_display && display.is_any_current())
        {
            return &display;
        }
        else if (!use_current_display && !display.is_any_current())
        {
            return &display;
        }
    }
    return nullptr;
}

} // namespace vipu
