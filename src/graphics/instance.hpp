#ifndef instance_hpp_vipu_graphics
#define instance_hpp_vipu_graphics

#include <memory>

#include "graphics/vulkan.hpp"
#include "graphics/physical_device.hpp"

namespace vipu
{

class Configuration;
class Device;

///
/// Instance
///   |
///   +--- Physical devices
///         |
///         +---- Displays


/// Owns:
///  - Vulkan instance
///  - Vulkan device
class Instance
{
public:
    Instance(Configuration *configuration);

    auto get()
    -> vk::Instance;

    void scan_instance_layers();

    void scan_global_instance_extensions();

    void create_instance();

    void scan_physical_devices();

    auto choose_physical_device()
    -> Physical_device &;

private:
    struct Layer_info
    {
        std::vector<vk::ExtensionProperties> extension_properties;
    };

    Configuration                       *m_configuration;
    std::vector<vk::LayerProperties>     m_instance_layer_properties;
    std::vector<Layer_info>              m_instance_layer_info;
    std::vector<vk::ExtensionProperties> m_global_extension_properties;
    vk::UniqueInstance                   m_vk_instance;
    std::vector<Physical_device>         m_physical_devices;
};

} // namespace vipu

#endif // instance_hpp_vipu_graphics
