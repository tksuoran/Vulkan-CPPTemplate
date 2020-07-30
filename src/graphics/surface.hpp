#ifndef surface_hpp_vipu_graphics
#define surface_hpp_vipu_graphics

#include "graphics/vulkan.hpp"

namespace vipu
{

class Configuration;
class Instance;
class Physical_device;

auto surface_format_score(vk::Format format)
-> int;

class Surface
{
public:
    enum Type
    {
        eNone = 0,
        eXCB,
        eDisplay
    };

    Surface(Configuration   *configuration,
            Instance        *instance,
            Physical_device *physical_device);

    auto choose_format()
    -> vk::SurfaceFormatKHR;

    auto get()
    -> vk::SurfaceKHR;

protected:
    Configuration      *m_configuration  {nullptr};
    Instance           *m_instance       {nullptr};
    Physical_device    *m_physical_device{nullptr};
    vk::Instance        m_vk_instance;
    vk::PhysicalDevice  m_vk_physical_device;

    bool                 m_quit {false};
    bool                 m_pause{false};
    vk::UniqueSurfaceKHR m_vk_surface;
    vk::SurfaceFormatKHR m_surface_format;
};

} // namespace vipu

#endif // surface_hpp_vipu_graphics
