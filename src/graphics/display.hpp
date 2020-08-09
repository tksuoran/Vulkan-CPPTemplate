#ifndef display_hpp_vipu_graphics
#define display_hpp_vipu_graphics

#include <vector>
#include <cstdint>
#include <limits>

#include "graphics/vulkan.hpp"

namespace vipu
{

class Physical_device;

struct Plane
{
public:
    Plane() = default;

    Plane(vk::PhysicalDevice              physical_device,
          vk::DisplayKHR                  display,
          uint32_t                        plane_index,
          vk::DisplayPlaneProperties2KHR &properties);

    std::vector<vk::DisplayKHR>     supported_displays;
    vk::DisplayPlaneProperties2KHR  properties;
    bool                            current;
    bool                            supported;
};

class Display
{
public:
    Display(vk::PhysicalDevice         physical_device,
            uint32_t                   display_index,
            vk::DisplayProperties2KHR &properties);

    auto get()
    -> vk::DisplayKHR;

    auto get_properties()
    -> const vk::DisplayProperties2KHR &;

    auto get_mode_properties()
    -> const std::vector<vk::DisplayModeProperties2KHR> &;

    auto get_plane_index()
    -> uint32_t;

    auto is_any_current()
    -> bool;

private:
    vk::DisplayKHR                              m_vk_display;
    vk::DisplayProperties2KHR                   m_properties;
    std::vector<vk::DisplayModeProperties2KHR>  m_mode_properties;
    std::vector<Plane>                          m_planes;
    bool                                        m_is_any_current{false};

    uint32_t                                    m_display_index      {std::numeric_limits<uint32_t>::max()};
    uint32_t                                    m_display_plane_index{std::numeric_limits<uint32_t>::max()};
};

} // namespace vipu

#endif // display_hpp_vipu_graphics
