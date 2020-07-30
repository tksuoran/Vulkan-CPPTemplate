#include "gsl/gsl"

#include "graphics/instance.hpp"
#include "graphics/configuration.hpp"
#include "graphics/log.hpp"

namespace vipu
{

Instance::Instance(Configuration *configuration)
:   m_configuration(configuration)
{
    uint32_t api_version = vk::enumerateInstanceVersion();
    log_vulkan.info("Vulkan major {}.{}.{}\n",
                    VK_VERSION_MAJOR(api_version),
                    VK_VERSION_MINOR(api_version),
                    VK_VERSION_PATCH(api_version));

    scan_instance_layers();
    scan_global_instance_extensions();
    create_instance();
    scan_physical_devices();
}

auto Instance::get()
-> vk::Instance
{
    Expects(m_vk_instance);
    return m_vk_instance.get();
}

void Instance::scan_instance_layers()
{
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
}

void Instance::scan_global_instance_extensions()
{
    m_global_extension_properties = vk::enumerateInstanceExtensionProperties();
    for (auto &extension : m_global_extension_properties)
    {
        std::string extensionName(extension.extensionName.data());
        log_vulkan.trace("Global extension {}\n", extensionName);
    }
}

void Instance::create_instance()
{
    vk::ApplicationInfo application_info{
        "application name",
        0,
        "engine name",
        0,
        VK_MAKE_VERSION(1, 1, 0) // TODO 1.2
    };

    std::array<char const *, 1> layer_names = {
        "VK_LAYER_KHRONOS_validation"
    };

    // Deprecated by VK_EXT_DEBUG_UTILS
    //  - VK_EXT_DEBUG_REPORT

    // Promoted to Vulkan 1.2:
    //  - VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2

    std::array<char const *, 8> display_instance_extension_names = {
        VK_KHR_DISPLAY_EXTENSION_NAME,
        VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
        VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
        VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME
    };

    std::array<char const *, 5> xcb_instance_extension_names = {
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME
    };

    // VK_EXT_validation_features
    std::array<vk::ValidationFeatureEnableEXT, 3> validation_feature_enable{
        // GPU-assisted validation is enabled. Activating this feature
        // instruments shader programs to generate additional diagnostic data.
        vk::ValidationFeatureEnableEXT::eGpuAssisted,

        // The validation layers reserve a descriptor set binding slot for
        // their own use. The layer reports a value for
        // VkPhysicalDeviceLimits::maxBoundDescriptorSets that is one less
        // than the value reported by the device. If the device supports the
        // binding of only one descriptor set, the validation layer does not
        // perform GPU-assisted validation.
        vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot,

        // Vulkan best-practices validation is enabled. Activating this feature
        // enables the output of warnings related to common misuse of the API,
        // but which are not explicitly prohibited by the specification.
        vk::ValidationFeatureEnableEXT::eBestPractices

        // The layers will process debugPrintfEXT operations in shaders and
        // send the resulting output to the debug callback.
        //vk::ValidationFeatureEnableEXT::eDebugPrintf
    };
    vk::StructureChain<vk::InstanceCreateInfo,
                        vk::ValidationFeaturesEXT
    > instance_create_info{
        vk::InstanceCreateInfo{
            vk::InstanceCreateFlags(),
            &application_info,
            layer_names.size(),
            layer_names.data(),
            xcb_instance_extension_names.size(),
            xcb_instance_extension_names.data()
        },
        vk::ValidationFeaturesEXT{
            validation_feature_enable.size(),
            validation_feature_enable.data(),
            0,
            nullptr
        }
    };

    m_vk_instance = vk::createInstanceUnique(instance_create_info.get<vk::InstanceCreateInfo>());

    log_vulkan.trace("{} completed\n", __func__);

    Ensures(m_vk_instance);
}

void Instance::scan_physical_devices()
{
    Expects(m_vk_instance);

    auto physical_devices = m_vk_instance->enumeratePhysicalDevices();

    log_vulkan.trace("Found {} physical devices\n", m_physical_devices.size());

    // Scan all physical devices
    m_physical_devices.resize(physical_devices.size());
    for (size_t physical_device_index = 0;
         physical_device_index < physical_devices.size();
         ++physical_device_index)
    {
        m_physical_devices[physical_device_index] = Physical_device(m_configuration,
                                                                    this,
                                                                    physical_devices[physical_device_index]);
    }

    log_vulkan.trace("{} completed\n", __func__);

    Ensures(m_physical_devices.size() > 0);
}

auto Instance::choose_physical_device()
-> Physical_device &
{
    Expects(!m_physical_devices.empty());

    size_t physical_device_index = 0; // TODO
    log_vulkan.info("Chose physical device {}\n", physical_device_index);
    return m_physical_devices[physical_device_index];
}

} // namespace vipu
