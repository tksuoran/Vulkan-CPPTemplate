#include <gsl/gsl>

#include "graphics/surface.hpp"
#include "graphics/instance.hpp"
#include "graphics/log.hpp"

namespace vipu
{

auto surface_format_score(vk::Format format)
-> int
{
    switch (format)
    {
        case vk::Format::eR16G16B16A16Sfloat:     return 10;
        case vk::Format::eA2B10G10R10UnormPack32: return 9;
        case vk::Format::eA2R10G10B10UnormPack32: return 9;
        case vk::Format::eA8B8G8R8UnormPack32:    return 8;
        case vk::Format::eA8B8G8R8SrgbPack32:     return 7;
        case vk::Format::eR8G8B8A8Unorm:          return 8;
        case vk::Format::eB8G8R8A8Unorm:          return 8;
        case vk::Format::eA1R5G5B5UnormPack16:    return 6;
        default:                                  return 0;
    }
}

Surface::Surface(Configuration   *configuration,
                 Instance        *instance,
                 Physical_device *physical_device)
    : m_configuration     {configuration}
    , m_instance          {instance}
    , m_physical_device   {physical_device}
    , m_vk_instance       {instance->get()}
    , m_vk_physical_device{physical_device->get()}
{
    Expects(configuration   != nullptr);
    Expects(instance        != nullptr);
    Expects(physical_device != nullptr);
    Expects(m_vk_instance);
    Expects(m_vk_physical_device);
}

auto Surface::choose_format()
-> vk::SurfaceFormatKHR
{
    Expects(m_vk_physical_device);
    Expects(m_vk_surface);

    auto surface_formats = m_vk_physical_device.getSurfaceFormatsKHR(m_vk_surface.get());
    vk::SurfaceFormatKHR best_format;
    int max_score{0};
    for (auto format : surface_formats)
    {
        int score = surface_format_score(format.format);
        log_vulkan.trace("\tFormat {} colorspace {} - score {}\n",
                            vk::to_string(format.format),
                            vk::to_string(format.colorSpace),
                            score);
        if (score > max_score)
        {
            best_format = format.format;
            max_score = score;
        }
    }
    log_vulkan.trace("Chose format {} colorspace {}\n",
                     vk::to_string(best_format.format),
                     vk::to_string(best_format.colorSpace));

    VERIFY(best_format != vk::Format::eUndefined);
    return best_format;
}

auto Surface::get()
-> vk::SurfaceKHR
{
    return m_vk_surface.get();
}


} // namespace vipu
