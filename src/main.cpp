#include <cstdio>
#include <cstdlib>
#include "log.hpp"
#include "fmt/format.h"
#include "vulkan/vulkan.hpp"

using vipu::log::Log;

Log::Category log_vulkan(Log::Color::GREEN, Log::Color::GRAY, Log::Level::LEVEL_TRACE);

class Vulkan
{
public:
    struct LayerInfo
    {
        std::vector<vk::ExtensionProperties> extension_properties;
    };

    std::vector<vk::LayerProperties>     m_instance_layer_properties;
    std::vector<LayerInfo>               m_instance_layer_info;
    std::vector<vk::ExtensionProperties> m_global_extension_properties;

    Vulkan()
    {
        uint32_t api_version = vk::enumerateInstanceVersion();
        log_vulkan.info("Vulkan major {}.{}.{}\n",
                        VK_VERSION_MAJOR(api_version),
                        VK_VERSION_MINOR(api_version),
                        VK_VERSION_PATCH(api_version));

        m_instance_layer_properties = vk::enumerateInstanceLayerProperties();
        m_instance_layer_info.resize(m_instance_layer_properties.size());
        for (size_t i = 0; i < m_instance_layer_properties.size(); ++i)
        {
            auto &layer = m_instance_layer_properties[i];
            auto &info = m_instance_layer_info[i];
            std::string layerName(layer.layerName.data());
            log_vulkan.trace("Instance layer {}\n", layerName);
            info.extension_properties = vk::enumerateInstanceExtensionProperties(layerName);
            for (auto &extension : info.extension_properties)
            {
                std::string extensionName(extension.extensionName.data());
                log_vulkan.trace("\tInstance layer extension {}\n", extensionName);
            }
        }

        m_global_extension_properties = vk::enumerateInstanceExtensionProperties();
        for (auto &extension : m_global_extension_properties)
        {
            std::string extensionName(extension.extensionName.data());
            log_vulkan.trace("Global extension {}\n", extensionName);
        }

        vk::ApplicationInfo application_info{
            "application name",
            0,
            "engine name",
            VK_MAKE_VERSION(1, 2, 0)
        };

        char const *layer_names[1] = {
            "VK_LAYER_KHRONOS_validation"
        };

        char const *extension_names[2] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_DISPLAY_EXTENSION_NAME
        };

        vk::InstanceCreateInfo instance_create_info{
            vk::InstanceCreateFlags(),
            &application_info,
            0, //1,
            nullptr, // layer_names,
            2,
            extension_names
        };

        m_instance = vk::createInstance(instance_create_info);

        m_physical_devices = m_instance.enumeratePhysicalDevices();

        log_vulkan.trace("Found {} physical devices\n", m_physical_devices.size());

        //for (auto &physical_device : m_physical_devices)
        m_physical_device = m_physical_devices.front();

        m_device_extensions = m_physical_device.enumerateDeviceExtensionProperties();
        log_vulkan.trace("\tFound {} device extensions\n", m_device_extensions.size());
        for (auto &extension : m_device_extensions)
        {
            std::string extension_name{extension.extensionName.data()};
            log_vulkan.trace("\tFound extension {}\n", extension_name);
        }

        m_physical_device_properties = m_physical_device.getProperties();
        m_queue_family_properties    = m_physical_device.getQueueFamilyProperties();
        m_device_features            = m_physical_device.getFeatures();
        m_display_properties         = m_physical_device.getDisplayPropertiesKHR();
        log_vulkan.trace("Found {} displays\n", m_display_properties.size());
        m_display_info.resize(m_display_properties.size());
        for (size_t display_index = 0;
             display_index < m_display_properties.size();
             ++display_index)
        {
            auto &properties   = m_display_properties[display_index];
            auto &display_info = m_display_info[display_index];
            std::string name{properties.displayName};
            log_vulkan.trace("Display {}: {}\n", display_index, name);
            display_info.mode_properties = m_physical_device.getDisplayModeProperties2KHR(properties.display);
            log_vulkan.trace("\tModes: {}\n", display_info.mode_properties.size());
            for (size_t mode_index = 0;
                 mode_index < display_info.mode_properties.size();
                 ++mode_index)
            {
                auto &mode_parameters = display_info.mode_properties[mode_index].displayModeProperties.parameters;
                log_vulkan.trace("\t\tMode {}: {} x {} @ {} Hz\n",
                                 mode_index,
                                 mode_parameters.visibleRegion.width,
                                 mode_parameters.visibleRegion.height,
                                 mode_parameters.refreshRate);
            }

            m_display_mode = display_info.mode_properties.front().displayModeProperties.displayMode;

            display_info.plane_properties = m_physical_device.getDisplayPlaneProperties2KHR();
            log_vulkan.trace("\tPlanes: {}\n", display_info.plane_properties.size());
            display_info.plane_info.resize(display_info.plane_properties.size());
            m_display_plane_index = std::numeric_limits<uint32_t>::max();
            for (uint32_t plane_index = 0;
                 plane_index < display_info.plane_properties.size();
                 ++plane_index)
            {
                auto &plane      = display_info.plane_properties[plane_index];
                auto &plane_info = display_info.plane_info[plane_index];

                vk::DisplayPlaneInfo2KHR plane_info_key{m_display_mode, plane_index};
                plane_info.supported_displays = m_physical_device.getDisplayPlaneSupportedDisplaysKHR(plane_index);
                plane_info.capabilities       = m_physical_device.getDisplayPlaneCapabilities2KHR(plane_info_key);

                bool current = plane.displayPlaneProperties.currentDisplay == properties.display;
                bool supported = std::find(plane_info.supported_displays.begin(),
                                           plane_info.supported_displays.end(),
                                           properties.display) != plane_info.supported_displays.end();

                log_vulkan.trace("\t\tPlane {}: current display: {}, display supported: {}, alpha: {}\n",
                                 plane_index,
                                 current   ? "yes" : "no",
                                 supported ? "yes" : "no",
                                 vk::to_string(plane_info.capabilities.capabilities.supportedAlpha));

                if (current && supported && (m_display_plane_index == std::numeric_limits<uint32_t>::max()))
                {
                    m_display_plane_index = plane_index;
                }
            }
        }
        m_display_index = 0;
        VERIFY(m_display_plane_index != std::numeric_limits<uint32_t>::max());

        auto extent = m_display_info[m_display_index].plane_info[m_display_plane_index].capabilities.capabilities.maxDstExtent;

        vk::DisplaySurfaceCreateInfoKHR surface_create_info {
            vk::DisplaySurfaceCreateFlagsKHR(),
            m_display_mode,
            m_display_plane_index,
            0, // stack index
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            1.0f, // global alpha
            vk::DisplayPlaneAlphaFlagBitsKHR::eOpaque,
            extent
        };

        m_surface = m_instance.createDisplayPlaneSurfaceKHR(surface_create_info);
        log_vulkan.trace("Created surface {} x {} on plane index {}\n",
                         extent.width,
                         extent.height,
                         m_display_plane_index);

        m_instance.destroySurfaceKHR(m_surface);
    }

    ~Vulkan()
    {
        m_instance.destroy();
    }

    vk::Instance                           m_instance;
    std::vector<vk::PhysicalDevice>        m_physical_devices;
    std::vector<vk::ExtensionProperties>   m_device_extensions;
    vk::PhysicalDevice                     m_physical_device;
    vk::PhysicalDeviceProperties           m_physical_device_properties;
    std::vector<vk::QueueFamilyProperties> m_queue_family_properties;
    vk::PhysicalDeviceFeatures             m_device_features;

    std::vector<vk::DisplayPropertiesKHR>  m_display_properties;
    struct Plane_info
    {
        std::vector<vk::DisplayKHR>      supported_displays;
        vk::DisplayPlaneCapabilities2KHR capabilities;
    };
    struct Display_info
    {
        std::vector<vk::DisplayModeProperties2KHR>  mode_properties;
        std::vector<vk::DisplayPlaneProperties2KHR> plane_properties;
        std::vector<Plane_info>                     plane_info;
    };
    std::vector<Display_info>               m_display_info;

    vk::DisplayModeKHR                      m_display_mode;
    uint32_t                                m_display_index{0};
    uint32_t                                m_display_plane_index{0};

    vk::SurfaceKHR                          m_surface;
};

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    Vulkan vulkan;

    return 0;
}
