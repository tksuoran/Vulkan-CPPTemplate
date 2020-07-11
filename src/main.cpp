#include <cstdio>
#include <cstdlib>
#include "fmt/format.h"
#include "vulkan/vulkan.hpp"

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
        fmt::print("Vulkan major {}.{}.{}\n",
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
            fmt::print("Instance layer {}\n", layerName);
            info.extension_properties = vk::enumerateInstanceExtensionProperties(layerName);
            for (auto &extension : info.extension_properties)
            {
                std::string extensionName(extension.extensionName.data());
                fmt::print("\tInstance layer extension {}\n", extensionName);
            }
        }

        m_global_extension_properties = vk::enumerateInstanceExtensionProperties();
        for (auto &extension : m_global_extension_properties)
        {
            std::string extensionName(extension.extensionName.data());
            fmt::print("Global extension {}\n", extensionName);
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
            1,
            layer_names,
            2,
            extension_names
        };

        m_instance = vk::createInstance(instance_create_info);

        m_physical_devices = m_instance.enumeratePhysicalDevices();

        fmt::print("Found {} physical devices\n", m_physical_devices.size());

        //for (auto &physical_device : m_physical_devices)
        m_device = m_physical_devices.front();

        m_device_extensions = m_device.enumerateDeviceExtensionProperties();
        fmt::print("\tFound {} device extensions\n", m_device_extensions.size());
        for (auto &extension : m_device_extensions)
        {
            std::string extension_name{extension.extensionName.data()};
            fmt::print("\tFound extension {}\n", extension_name);
        }

        m_device_properties       = m_device.getProperties();
        m_queue_family_properties = m_device.getQueueFamilyProperties();
        m_device_features         = m_device.getFeatures();
        m_display_properties      = m_device.getDisplayPropertiesKHR();
        fmt::print("Found {} displays\n", m_display_properties.size());
        m_display_info.resize(m_display_properties.size());
        for (size_t i = 0; i < m_display_properties.size(); ++i)
        {
            auto &properties = m_display_properties[i];
            auto &info = m_display_info[i];
            std::string name{properties.displayName};
            fmt::print("Display {}: {}\n", i, name);
            info.mode_properties = m_device.getDisplayModePropertiesKHR(properties.display);
            fmt::print("\tModes: {}\n", info.mode_properties.size());
            for (size_t j = 0; j < info.mode_properties.size(); ++j)
            {
                auto &mode = info.mode_properties[j].parameters;
                fmt::print("\t\tMode {}: {} x {} @ {} Hz\n",
                           j,
                           mode.visibleRegion.width,
                           mode.visibleRegion.height,
                           mode.refreshRate);
            }

            m_display_mode = info.mode_properties.front().displayMode;
            //m_display_mode = static_cast<VkDisplayModeKHR>(info.mode_properties.front().displayMode);

            info.plane_properties = m_device.getDisplayPlanePropertiesKHR();
            fmt::print("\tPlanes:     {}\n", info.plane_properties.size());
            for (size_t j = 0; j < info.plane_properties.size(); ++j)
            {
                auto supported_displays = m_device.getDisplayPlaneSupportedDisplaysKHR(j);
                auto plane_capabilities = m_device.getDisplayPlaneCapabilitiesKHR(m_display_mode, j);

                auto &plane = info.plane_properties[j];

                bool current = plane.currentDisplay == properties.display;
                bool supported = std::find(supported_displays.begin(),
                                           supported_displays.end(),
                                           properties.display) != supported_displays.end();

                fmt::print("\t\tPlane {}: current display: {}, display supported: {}, alpha: {}\n",
                            j,
                            current   ? "yes" : "no",
                            supported ? "yes" : "no",
                            vk::to_string(plane_capabilities.supportedAlpha));
            }
        }
    }

    ~Vulkan()
    {
        m_instance.destroy();
    }

    vk::Instance                           m_instance;
    std::vector<vk::PhysicalDevice>        m_physical_devices;
    std::vector<vk::ExtensionProperties>   m_device_extensions;
    vk::PhysicalDevice                     m_device;
    vk::PhysicalDeviceProperties           m_device_properties;
    std::vector<vk::QueueFamilyProperties> m_queue_family_properties;
    vk::PhysicalDeviceFeatures             m_device_features;

    std::vector<vk::DisplayPropertiesKHR>  m_display_properties;
    struct Display_info
    {
        std::vector<vk::DisplayModePropertiesKHR>   mode_properties;
        std::vector<vk::DisplayPlanePropertiesKHR>  plane_properties;
    };
    std::vector<Display_info>              m_display_info;

    vk::DisplayModeKHR                     m_display_mode;
};

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    Vulkan vulkan;

    return 0;
}
