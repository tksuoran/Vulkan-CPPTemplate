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
            0,
            VK_MAKE_VERSION(1, 2, 0)
        };

        std::array<char const *, 1> layer_names = {
            "VK_LAYER_KHRONOS_validation"
        };

        std::array<char const *, 10> instance_extension_names = {
            VK_KHR_DISPLAY_EXTENSION_NAME,
            VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
            VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
            VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
            VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME
        };

        vk::InstanceCreateInfo instance_create_info{
            vk::InstanceCreateFlags(),
            &application_info,
            layer_names.size(),
            layer_names.data(),
            instance_extension_names.size(),
            instance_extension_names.data()
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

        m_physical_device_properties = m_physical_device.getProperties2();
        m_queue_family_properties    = m_physical_device.getQueueFamilyProperties2();
        m_device_features            = m_physical_device.getFeatures2();

        m_display_properties         = m_physical_device.getDisplayProperties2KHR();
        log_vulkan.trace("Found {} displays\n", m_display_properties.size());
        m_display_info.resize(m_display_properties.size());
        for (size_t display_index = 0;
             display_index < m_display_properties.size();
             ++display_index)
        {
            auto &properties   = m_display_properties[display_index];
            auto &display_info = m_display_info[display_index];
            std::string name{properties.displayProperties.displayName};
            log_vulkan.trace("Display {}: {}\n", display_index, name);
            display_info.mode_properties = m_physical_device.getDisplayModeProperties2KHR(properties.displayProperties.display);
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

                bool current = plane.displayPlaneProperties.currentDisplay == properties.displayProperties.display;
                bool supported = std::find(plane_info.supported_displays.begin(),
                                           plane_info.supported_displays.end(),
                                           properties.displayProperties.display) != plane_info.supported_displays.end();

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

        m_surface = m_instance.createDisplayPlaneSurfaceKHR(surface_create_info);
        log_vulkan.trace("Created surface {} x {} on plane index {}\n",
                         extent.width,
                         extent.height,
                         m_display_plane_index);

        uint32_t graphics_queue_family_index = std::numeric_limits<uint32_t>::max();
        uint32_t present_queue_family_index  = std::numeric_limits<uint32_t>::max();
        bool use_single_queue_family;
        for (uint32_t queue_family_index = 0;
             queue_family_index < m_queue_family_properties.size();
             ++queue_family_index)
        {
            bool present_supported = m_physical_device.getSurfaceSupportKHR(queue_family_index, m_surface);
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

        std::array<const float, 1> priorities {0.0f};

        vk::DeviceQueueCreateInfo device_queue_create_info{
            vk::DeviceQueueCreateFlags(),
            graphics_queue_family_index,
            1,
            priorities.data()
        };

        // Enable everything supported by NVIDIA
        vk::PhysicalDeviceFeatures features;
        features.setRobustBufferAccess                     (VK_TRUE);
        features.setFullDrawIndexUint32                    (VK_TRUE);
        features.setImageCubeArray                         (VK_TRUE);
        features.setIndependentBlend                       (VK_TRUE);
        features.setGeometryShader                         (VK_TRUE);
        features.setTessellationShader                     (VK_TRUE);
        features.setSampleRateShading                      (VK_TRUE);
        features.setDualSrcBlend                           (VK_TRUE);
        features.setLogicOp                                (VK_TRUE);
        features.setMultiDrawIndirect                      (VK_TRUE);
        features.setDrawIndirectFirstInstance              (VK_TRUE);
        features.setDepthClamp                             (VK_TRUE);
        features.setDepthBiasClamp                         (VK_TRUE);
        features.setFillModeNonSolid                       (VK_TRUE);
        features.setDepthBounds                            (VK_TRUE);
        features.setWideLines                              (VK_TRUE);
        features.setLargePoints                            (VK_TRUE);
        features.setAlphaToOne                             (VK_TRUE);
        features.setMultiViewport                          (VK_TRUE);
        features.setSamplerAnisotropy                      (VK_TRUE);
        features.setTextureCompressionETC2                 (VK_FALSE);
        features.setTextureCompressionASTC_LDR             (VK_FALSE);
        features.setTextureCompressionBC                   (VK_TRUE);
        features.setOcclusionQueryPrecise                  (VK_TRUE);
        features.setPipelineStatisticsQuery                (VK_TRUE);
        features.setVertexPipelineStoresAndAtomics         (VK_TRUE);
        features.setFragmentStoresAndAtomics               (VK_TRUE);
        features.setShaderTessellationAndGeometryPointSize (VK_TRUE);
        features.setShaderImageGatherExtended              (VK_TRUE);
        features.setShaderStorageImageExtendedFormats      (VK_TRUE);
        features.setShaderStorageImageMultisample          (VK_TRUE);
        features.setShaderStorageImageReadWithoutFormat    (VK_TRUE);
        features.setShaderStorageImageWriteWithoutFormat   (VK_TRUE);
        features.setShaderUniformBufferArrayDynamicIndexing(VK_TRUE);
        features.setShaderSampledImageArrayDynamicIndexing (VK_TRUE);
        features.setShaderStorageBufferArrayDynamicIndexing(VK_TRUE);
        features.setShaderStorageImageArrayDynamicIndexing (VK_TRUE);
        features.setShaderClipDistance                     (VK_TRUE);
        features.setShaderCullDistance                     (VK_TRUE);
        features.setShaderFloat64                          (VK_TRUE);
        features.setShaderInt64                            (VK_TRUE);
        features.setShaderInt16                            (VK_TRUE);
        features.setShaderResourceResidency                (VK_TRUE);
        features.setShaderResourceMinLod                   (VK_TRUE);
        features.setSparseBinding                          (VK_TRUE);
        features.setSparseResidencyBuffer                  (VK_TRUE);
        features.setSparseResidencyImage2D                 (VK_TRUE);
        features.setSparseResidencyImage3D                 (VK_TRUE);
        features.setSparseResidency2Samples                (VK_TRUE);
        features.setSparseResidency4Samples                (VK_TRUE);
        features.setSparseResidency8Samples                (VK_TRUE);
        features.setSparseResidency16Samples               (VK_TRUE);
        features.setSparseResidencyAliased                 (VK_TRUE);
        features.setVariableMultisampleRate                (VK_TRUE);
        features.setInheritedQueries                       (VK_TRUE);

        // std::array<char const *, 1> device_extension_names = {
        //     VK_KHR_SURFACE_EXTENSION_NAME
        // };

        vk::DeviceCreateInfo device_create_info{
            vk::DeviceCreateFlags(),
            1,
            &device_queue_create_info,
            layer_names.size(),
            layer_names.data(),
            0,
            nullptr,
            &features
        };

        m_device = m_physical_device.createDevice(device_create_info);

        m_device.destroy();

        m_instance.destroySurfaceKHR(m_surface);
    }

    ~Vulkan()
    {
        m_instance.destroy();
    }

    vk::Instance                            m_instance;
    std::vector<vk::PhysicalDevice>         m_physical_devices;
    std::vector<vk::ExtensionProperties>    m_device_extensions;
    vk::PhysicalDevice                      m_physical_device;
    vk::PhysicalDeviceProperties2           m_physical_device_properties;
    std::vector<vk::QueueFamilyProperties2> m_queue_family_properties;
    vk::PhysicalDeviceFeatures2             m_device_features;

    std::vector<vk::DisplayProperties2KHR>  m_display_properties;
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

    vk::Device                              m_device;
};

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    Vulkan vulkan;

    return 0;
}
