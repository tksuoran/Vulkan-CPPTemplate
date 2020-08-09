#include <gsl/gsl>

#include "log/log.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "graphics/log.hpp"
#include "graphics/surface.hpp"

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
        case vk::Format::eA8B8G8R8UnormPack32:    return 11; // 8
        case vk::Format::eA8B8G8R8SrgbPack32:     return 11; // 8
        case vk::Format::eR8G8B8A8Unorm:          return 11; // 8
        case vk::Format::eB8G8R8A8Unorm:          return 11; // 8
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

Swapchain::Swapchain(Context &context)
{
    Expects(context.vk_device);
    Expects(context.vk_surface);
    Expects(context.surface != nullptr);
    Expects(context.graphics_queue_family_index != std::numeric_limits<uint32_t>::max());

    auto &surface_capabilities = context.surface->get_capabilities();
    auto surface_formats = context.vk_physical_device.getSurfaceFormatsKHR(context.vk_surface);
    VERIFY(!surface_formats.empty());

    m_surface_format = choose_format(surface_formats);
    VERIFY(m_surface_format.format != vk::Format::eUndefined);

    std::array<uint32_t, 1> queue_family_indices {
        context.graphics_queue_family_index
    };

    auto &c = surface_capabilities;
    log_vulkan.trace("minImageCount           {}\n",     c.minImageCount);
    log_vulkan.trace("maxImageCount           {}\n",     c.maxImageCount);
    log_vulkan.trace("currentExtent           {}, {}\n", c.currentExtent.width,  c.currentExtent.height);
    log_vulkan.trace("minImageExtent          {}, {}\n", c.minImageExtent.width, c.minImageExtent.height);
    log_vulkan.trace("maxImageExtent          {}, {}\n", c.maxImageExtent.width, c.maxImageExtent.height);
    log_vulkan.trace("maxImageArrayLayers     {}\n",     c.maxImageArrayLayers);
    log_vulkan.trace("supportedTransforms     {}\n", vk::to_string(c.supportedTransforms));
    log_vulkan.trace("currentTransform        {}\n", vk::to_string(c.currentTransform));
    log_vulkan.trace("supportedCompositeAlpha {}\n", vk::to_string(c.supportedCompositeAlpha));
    log_vulkan.trace("supportedUsageFlags     {}\n", vk::to_string(c.supportedUsageFlags));

    vk::Bool32 clipped = (context.surface_type == Surface::Type::eXCB) ? VK_TRUE : VK_FALSE;

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        vk::SwapchainCreateFlagsKHR{},              // flags
        context.vk_surface,                         // surface
        surface_capabilities.minImageCount,         // min image count
        m_surface_format.format,                    // image format
        m_surface_format.colorSpace,                // image color space
        surface_capabilities.currentExtent,         // extent
        1,                                          // array layers
        vk::ImageUsageFlagBits::eColorAttachment,   // image usage
        vk::SharingMode::eExclusive,                // sharing mode
        queue_family_indices.size(),                // queue family index count
        queue_family_indices.data(),                // queue family indices
        surface_capabilities.currentTransform,      // pre transform
        vk::CompositeAlphaFlagBitsKHR::eOpaque,     // composite alpha
        vk::PresentModeKHR::eFifo,                  // present mode
        clipped,                                    // clipped
        {} // m_vk_swapchain.get()
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
    m_vk_swapchain = context.vk_device.createSwapchainKHRUnique(swapchain_create_info);

    Ensures(m_vk_swapchain);
}

auto Swapchain::get()
-> vk::SwapchainKHR
{
    return m_vk_swapchain.get();
}

auto Swapchain::get_surface_format()
-> vk::SurfaceFormatKHR
{
    return m_surface_format;
}


} // namespace vipu
