#ifndef display_surface_hpp_vipu_graphics
#define display_surface_hpp_vipu_graphics

#include <cstdint>
#include <limits>

#include "graphics/vulkan.hpp"
#include "graphics/surface.hpp"

namespace vipu
{

class Context;

class Display_surface
    : public Surface
{
public:
    Display_surface(Context &context);

private:
    vk::DisplayModeKHR  m_display_mode;
    uint32_t            m_display_plane_index{std::numeric_limits<uint32_t>::max()};
};

} // namespace vipu

#endif // display_surface_hpp_vipu_graphics
