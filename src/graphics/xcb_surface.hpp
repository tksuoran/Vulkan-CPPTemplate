#ifndef xcb_surface_hpp_vipu_graphics
#define xcb_surface_hpp_vipu_graphics

#include "graphics/surface.hpp"

namespace vipu
{

class XCB_surface
    : public Surface
{
public:
    XCB_surface(vk::Instance       vk_instance,
                vk::PhysicalDevice vk_physical_device);

private:
    void xcb_init_connection();

    void xcb_create_window_();

    void create_xcb_surface(vk::Instance instance, vk::PhysicalDevice vk_physical_device);

    void xcb_handle_event(const xcb_generic_event_t *event);

    void xcb_run();

    void update();

    xcb_window_t             m_xcb_window               {0};
    xcb_screen_t            *m_xcb_screen               {nullptr};
    xcb_connection_t        *m_xcb_connection           {nullptr};
    xcb_intern_atom_reply_t *m_xcb_delete_window_wm_atom{nullptr};
};

} // namespace vipu

#endif // xcb_surface_hpp_vipu_graphics
