#include <gsl/gsl>

#include "log/log.hpp"
#include "graphics/log.hpp"
#include "graphics/device.hpp"
#include "graphics/surface.hpp"
#include "graphics/swapchain.hpp"

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

auto choose_format(const std::vector<vk::SurfaceFormatKHR> &surface_formats)
-> vk::SurfaceFormatKHR
{
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

Swapchain::Swapchain(Device  *device,
                     Surface *surface)
{
    Expects(device != nullptr);
    Expects(surface != nullptr);

    auto vk_device = device->get();
    VERIFY(vk_device);

    auto vk_surface = surface->get();
    VERIFY(vk_surface);

    auto &surface_capabilities = surface->get_capabilities();
    auto surface_formats = device->get_surface_formats(vk_surface);
    VERIFY(!surface_formats.empty());

    auto surface_format = choose_format(surface_formats);

    std::array<uint32_t, 1> queue_family_indices {
        device->get_queue_family_indices().graphics
    };

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        vk::SwapchainCreateFlagsKHR{},              // flags
        vk_surface,                                  // surface
        surface_capabilities.minImageCount,         // min image count
        surface_format.format,                      // image format
        surface_format.colorSpace,                  // image color space
        surface_capabilities.currentExtent,         // extent
        1,                                          // array layers
        vk::ImageUsageFlagBits::eColorAttachment,   // image usage
        vk::SharingMode::eExclusive,                // sharing mode
        queue_family_indices.size(),                // queue family index count
        queue_family_indices.data(),                // queue family indices
        surface_capabilities.currentTransform,      // pre transform
        vk::CompositeAlphaFlagBitsKHR::eOpaque,     // composite alpha
        vk::PresentModeKHR::eFifo,                  // present mode
        VK_TRUE,                                    // clipped
        m_vk_swapchain.get()
    };

    // vk::SwapchainKHR swapchain;
    // vk::Result result = m_vk_device->createSwapchainKHR(&swapchain_create_info,
    //                                                     nullptr,
    //                                                     &swapchain);
    // printf("swapchain create result = %s\n", vk::to_string(result).c_str());
    // if (result != vk::Result::eSuccess) {
    //     abort();
    // }
    //
    // m_vk_swapchain = swapchain;
    m_vk_swapchain = vk_device.createSwapchainKHRUnique(swapchain_create_info);

}

auto Swapchain::get()
-> vk::SwapchainKHR
{
    return m_vk_swapchain.get();
}


} // namespace vipu
