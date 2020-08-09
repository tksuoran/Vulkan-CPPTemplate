#include "gsl/gsl"

#include "graphics/instance.hpp"
#include "graphics/context.hpp"
#include "graphics/log.hpp"

namespace vipu
{

auto debug_report_callback(
    VkDebugReportFlagsEXT      flags,
    VkDebugReportObjectTypeEXT objectType,
    uint64_t                   object,
    size_t                     location,
    int32_t                    messageCode,
    const char*                pLayerPrefix,
    const char*                pMessage,
    void*                      pUserData)
-> VkBool32
{
    auto *instance = reinterpret_cast<Instance*>(pUserData);
    if (instance != nullptr)
    {
        instance->debug_report_callback(static_cast<vk::DebugReportFlagsEXT>(flags),
                                        static_cast<vk::DebugReportObjectTypeEXT>(objectType),
                                        object,
                                        location,
                                        messageCode,
                                        pLayerPrefix,
                                        pMessage);
    }
    return VK_FALSE;
}

auto debug_utils_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
-> VkBool32
{
    auto *instance = reinterpret_cast<Instance*>(pUserData);
    if (instance != nullptr)
    {
        instance->debug_utils_messenger_callback(static_cast<vk::DebugUtilsMessageSeverityFlagsEXT>(messageSeverity),
                                                 static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageTypes),
                                                 reinterpret_cast<const vk::DebugUtilsMessengerCallbackDataEXT*>(pCallbackData));
    }
    return VK_FALSE;
}

void Instance::debug_report_callback(
    vk::DebugReportFlagsEXT      flags,
    vk::DebugReportObjectTypeEXT objectType,
    uint64_t                     object,
    size_t                       location,
    int32_t                      messageCode,
    const char*                  pLayerPrefix,
    const char*                  pMessage)
{
    log_vulkan.trace("flags {}, objectType {}, object {:x}, location {}, code {:x}, layer prefix {}, message {}\n\\",
                     vk::to_string(flags),
                     vk::to_string(objectType),
                     location,
                     messageCode,
                     pLayerPrefix,
                     (pMessage != nullptr) ? pMessage : "");
}

void Instance::debug_utils_messenger_callback(vk::DebugUtilsMessageSeverityFlagsEXT         messageSeverity,
                                              vk::DebugUtilsMessageTypeFlagsEXT             messageTypes,
                                              const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData)
{
    log_vulkan.trace("severity {}, types {}, id {} ({:x}), message {}\n",
                     vk::to_string(messageSeverity),
                     vk::to_string(messageTypes),
                     (pCallbackData->pMessageIdName != nullptr) ? pCallbackData->pMessageIdName : "",
                     pCallbackData->messageIdNumber,
                     (pCallbackData->pMessage != nullptr) ? pCallbackData->pMessage : "");
    if (pCallbackData->queueLabelCount > 0)
    {
        log_vulkan.trace("    queues ({})\n", pCallbackData->queueLabelCount);
        for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
        {
            auto &label = pCallbackData->pQueueLabels[i];
            log_vulkan.trace("        name {}, color ({}, {}, {}, {})\n",
                             (label.pLabelName != nullptr) ? label.pLabelName  : "",
                             label.color[0],
                             label.color[1],
                             label.color[2],
                             label.color[3]);
        }
    }
    if (pCallbackData->cmdBufLabelCount > 0)
    {
        log_vulkan.trace("    command buffers ({})\n", pCallbackData->cmdBufLabelCount);
        for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
        {
            auto &label = pCallbackData->pCmdBufLabels[i];
            log_vulkan.trace("        name {}, color ({}, {}, {}, {})\n",
                             (label.pLabelName != nullptr) ? label.pLabelName  : "",
                             label.color[0],
                             label.color[1],
                             label.color[2],
                             label.color[3]);
        }
    }
    if (pCallbackData->objectCount > 0)
    {
        log_vulkan.trace("    objects ({})\n", pCallbackData->objectCount);
        for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
        {
            auto &object = pCallbackData->pObjects[i];
            log_vulkan.trace("        type {}, handle {:x}, name {}\n",
                             vk::to_string(object.objectType),
                             object.objectHandle,
                             (object.pObjectName != nullptr) ? object.pObjectName : "");
        }
    }
}

Instance::Instance(Context &context)
{
    Expects(context.instance == nullptr);

    context.instance = this;

    auto fn_vkGetInstanceProcAddr = m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VERIFY(fn_vkGetInstanceProcAddr != nullptr);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(fn_vkGetInstanceProcAddr);

    uint32_t api_version = vk::enumerateInstanceVersion();
    log_vulkan.info("Vulkan major {}.{}.{}\n",
                    VK_VERSION_MAJOR(api_version),
                    VK_VERSION_MINOR(api_version),
                    VK_VERSION_PATCH(api_version));

    scan_instance_layers();
    scan_global_instance_extensions();
    create_instance(context);

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vk_instance.get());

    context.vk_instance = get();

    register_debug_report_callback();
    scan_physical_devices(context);

    Ensures(context.vk_instance);
    Ensures(context.instance);
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

void Instance::create_instance(Context &context)
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

    // Promoted to Vulkan 1.1:
    //  - VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2

    std::vector<const char*> instance_extension_names;
    instance_extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    instance_extension_names.emplace_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);
    instance_extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instance_extension_names.emplace_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    switch (context.surface_type)
    {
        case Surface::Type::eDisplay:
        {
            instance_extension_names.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
            instance_extension_names.emplace_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);
            instance_extension_names.emplace_back(VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME);
            instance_extension_names.emplace_back(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);
            break;
        }

        case Surface::Type::eXCB:
        {
            instance_extension_names.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
            break;
        }

        default:
        {
            FATAL("Bad surface type");
        }
    }

#if 0
    std::array<char const *, 9> display_instance_extension_names = {
        VK_KHR_DISPLAY_EXTENSION_NAME,
        VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
        VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,

        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME
    };

    instance_extension_names.emplace_back()
    std::array<char const *, 5> xcb_instance_extension_names = {
        VK_KHR_XCB_SURFACE_EXTENSION_NAME,                 // VK_KHR_xcb_surface

        VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,  // VK_KHR_get_surface_capabilities2
        VK_KHR_SURFACE_EXTENSION_NAME,                     // VK_KHR_surface
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME//,                 // VK_EXT_debug_utils
        VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME          // VK_EXT_validation_features
    };
#endif

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
    > instance_create_info_chain {
        vk::InstanceCreateInfo{
            vk::InstanceCreateFlags(),
            &application_info,
            layer_names.size(),
            layer_names.data(),
            static_cast<uint32_t>(instance_extension_names.size()),
            instance_extension_names.data()
        },
        vk::ValidationFeaturesEXT{
            validation_feature_enable.size(),
            validation_feature_enable.data(),
            0,
            nullptr
        }
    };

    vk::InstanceCreateInfo instance_create_info_basic {
        vk::InstanceCreateFlags(),
        &application_info,
        layer_names.size(),
        layer_names.data(),
        static_cast<uint32_t>(instance_extension_names.size()),
        instance_extension_names.data()
    };

    bool use_validation_features{true};

    vk::InstanceCreateInfo &instance_create_info = use_validation_features ? instance_create_info_chain.get<vk::InstanceCreateInfo>()
                                                                           : instance_create_info_basic;
    m_vk_instance = vk::createInstanceUnique(instance_create_info);

    log_vulkan.trace("{} completed\n", __func__);

    Ensures(m_vk_instance);
}

void Instance::register_debug_report_callback()
{
#if 0 // Does not link
    vk::DebugReportCallbackCreateInfoEXT create_info {
        vk::DebugReportFlagBitsEXT::eInformation        |
        vk::DebugReportFlagBitsEXT::eWarning            |
        vk::DebugReportFlagBitsEXT::ePerformanceWarning |
        vk::DebugReportFlagBitsEXT::eError              |
        vk::DebugReportFlagBitsEXT::eDebug,
        vipu::debug_report_callback,
        this
    };
    m_vk_debug_report_callback = m_vk_instance->createDebugReportCallbackEXTUnique(create_info);
#endif

#if 0
    VkDebugReportCallbackCreateInfoEXT create_info
    {
        VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
        nullptr,
        VK_DEBUG_REPORT_INFORMATION_BIT_EXT         |
        VK_DEBUG_REPORT_WARNING_BIT_EXT             |
        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
        VK_DEBUG_REPORT_ERROR_BIT_EXT               |
        VK_DEBUG_REPORT_DEBUG_BIT_EXT,
        vipu::debug_report_callback,
        this
    };
    PFN_vkVoidFunction pfn = vkGetInstanceProcAddr(m_vk_instance.get(), "vkCreateDebugReportCallbackEXT");
    auto CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(pfn);
    if (CreateDebugReportCallback != nullptr)
    {
        VkResult result = CreateDebugReportCallback(m_vk_instance.get(),
                                                    &create_info,
                                                    nullptr,
                                                    &m_vk_debug_report_callback);
        VERIFY(result == VK_SUCCESS);
    }
#endif

#if 0
    VkDebugUtilsMessengerCreateInfoEXT create_info
    {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        nullptr,
        0,                                                  // flags
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |   // messageSeverity
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |   // messageType
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        vipu::debug_utils_messenger_callback,
        this
    };
    PFN_vkVoidFunction pfn = vkGetInstanceProcAddr(m_vk_instance.get(), "vkCreateDebugUtilsMessengerEXT");
    auto CreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(pfn);
    if (CreateDebugUtilsMessenger != nullptr)
    {
        VkResult result = CreateDebugUtilsMessenger(m_vk_instance.get(),
                                                    &create_info,
                                                    nullptr,
                                                    &m_vk_debug_utils_messenger);
        VERIFY(result == VK_SUCCESS);
    }
#endif
    vk::DebugUtilsMessengerCreateInfoEXT create_info
    {
        {},                                                  // flags
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | // messageSeverity
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo    |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral     | // messageType
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation  |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        vipu::debug_utils_messenger_callback,
        this
    };
    // PFN_vkVoidFunction pfn = vkGetInstanceProcAddr(m_vk_instance.get(), "vkCreateDebugUtilsMessengerEXT");
    // auto CreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(pfn);
    // if (CreateDebugUtilsMessenger != nullptr)
    // {
    m_vk_debug_utils_messenger = m_vk_instance->createDebugUtilsMessengerEXTUnique(create_info);
}

void Instance::scan_physical_devices(Context &context)
{
    Expects(m_vk_instance);

    auto physical_devices = m_vk_instance->enumeratePhysicalDevices();

    log_vulkan.trace("Found {} physical devices\n", physical_devices.size());

    // Scan all physical devices
    m_physical_devices.resize(physical_devices.size());
    for (size_t physical_device_index = 0;
         physical_device_index < physical_devices.size();
         ++physical_device_index)
    {
        m_physical_devices[physical_device_index] = Physical_device(context,
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
    log_vulkan.info("Found {} physical devices\n", m_physical_devices.size());
    log_vulkan.info("Chose physical device {}\n", physical_device_index);

    return m_physical_devices[physical_device_index];
}

} // namespace vipu
