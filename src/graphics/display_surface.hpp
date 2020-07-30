#ifndef display_surface_hpp_vipu_graphics
#define display_surface_hpp_vipu_graphics

#include <cstdint>
#include <limits>

#include "graphics/vulkan.hpp"
#include "graphics/surface.hpp"

namespace vipu
{

class Configuration;
class Display;
class Instance;
class Physical_device;

/// Display surface
///  - specific Vulkan device
///  - specific Vulkan display
///  - specific Vulkan display mode
///  - specific Vulkan display plane
class Display_surface
    : public Surface
{
public:
    Display_surface(Configuration   *configuration,
                    Instance        *instance,
                    Physical_device *physical_device);

private:
    Display            *m_display;
    vk::DisplayKHR      m_vk_display;
    vk::DisplayModeKHR  m_display_mode;
    uint32_t            m_display_index      {std::numeric_limits<uint32_t>::max()};
    uint32_t            m_display_plane_index{std::numeric_limits<uint32_t>::max()};
};

} // namespace vipu

#endif // display_surface_hpp_vipu_graphics
