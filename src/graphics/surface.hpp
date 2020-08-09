#ifndef surface_hpp_vipu_graphics
#define surface_hpp_vipu_graphics

#include <vector>

#include "graphics/vulkan.hpp"

namespace vipu
{

class Context;

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

    // auto choose_format(Context &context)
    // -> vk::SurfaceFormatKHR;

    auto get()
    -> vk::SurfaceKHR;

    auto get_present_modes()
    -> const std::vector<vk::PresentModeKHR> &;

    auto get_capabilities()
    -> const vk::SurfaceCapabilitiesKHR &;

protected:
    void get_properties(Context &context);

    std::vector<vk::PresentModeKHR> m_present_modes;
    vk::SurfaceCapabilitiesKHR      m_surface_capabilities;

    bool                 m_quit {false};
    bool                 m_pause{false};
    vk::UniqueSurfaceKHR m_vk_surface;
    vk::SurfaceFormatKHR m_surface_format;
};

} // namespace vipu

#endif // surface_hpp_vipu_graphics
