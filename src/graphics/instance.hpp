#ifndef instance_hpp_vipu_graphics
#define instance_hpp_vipu_graphics

#include <memory>

#include "graphics/vulkan.hpp"
#include "graphics/physical_device.hpp"

namespace vipu
{

class Context;
class Device;

class Instance
{
public:
    Instance(Context &context);

    auto get()
    -> vk::Instance;

    auto choose_physical_device()
    -> Physical_device &;

    void debug_report_callback(
        vk::DebugReportFlagsEXT      flags,
        vk::DebugReportObjectTypeEXT objectType,
        uint64_t                     object,
        size_t                       location,
        int32_t                      messageCode,
        const char*                  pLayerPrefix,
        const char*                  pMessage);

    void debug_utils_messenger_callback(vk::DebugUtilsMessageSeverityFlagsEXT         messageSeverity,
                                        vk::DebugUtilsMessageTypeFlagsEXT             messageTypes,
                                        const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData);

private:
    void scan_instance_layers();

    void scan_global_instance_extensions();

    void create_instance(Context &context);

    void scan_physical_devices(Context &context);

    void register_debug_report_callback();
    struct Layer_info
    {
        std::vector<vk::ExtensionProperties> extension_properties;
    };

    vk::DynamicLoader                    m_dl;
    std::vector<vk::LayerProperties>     m_instance_layer_properties;
    std::vector<Layer_info>              m_instance_layer_info;
    std::vector<vk::ExtensionProperties> m_global_extension_properties;
    vk::UniqueInstance                   m_vk_instance;
    std::vector<Physical_device>         m_physical_devices;
    //vk::UniqueDebugReportCallbackEXT     m_vk_debug_report_callback;
    VkDebugReportCallbackEXT             m_vk_debug_report_callback;
    vk::UniqueDebugUtilsMessengerEXT     m_vk_debug_utils_messenger;
};

} // namespace vipu

#endif // instance_hpp_vipu_graphics
