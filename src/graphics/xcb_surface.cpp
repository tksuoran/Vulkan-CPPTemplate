#include <gsl/gsl>

#include "graphics/xcb_surface.hpp"
#include "graphics/configuration.hpp"
#include "graphics/log.hpp"

namespace vipu
{

XCB_surface::XCB_surface(vk::Instance       vk_instance,
                         vk::PhysicalDevice vk_physical_device)
{
    Expects(vk_instance);
    Expects(vk_physical_device);

    xcb_init_connection();
    xcb_create_window_();

    create_xcb_surface(vk_instance, vk_physical_device);
}

void XCB_surface::xcb_init_connection()
{
    const char *display_envar = getenv("DISPLAY");
    if ((display_envar == nullptr) || display_envar[0] == '\0') {
        FATAL("Environment variable DISPLAY requires a valid value.\nExiting ...\n");
    }

    int preferred_screen{0};
    m_xcb_connection = xcb_connect(nullptr, &preferred_screen);
    if (xcb_connection_has_error(m_xcb_connection) > 0)
    {
        FATAL("Cannot find a compatible Vulkan installable client driver "
              "(ICD).\nExiting ...\n");
    }

    const xcb_setup_t *setup = xcb_get_setup(m_xcb_connection);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    while (preferred_screen-- > 0)
    {
        xcb_screen_next(&iter);
    }

    m_xcb_screen = iter.data;
}

void XCB_surface::xcb_create_window_()
{
    m_xcb_window = xcb_generate_id(m_xcb_connection);

    uint32_t value_mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t value_list[32];
    value_list[0] = m_xcb_screen->black_pixel;
    value_list[1] = XCB_EVENT_MASK_KEY_RELEASE |
                    XCB_EVENT_MASK_EXPOSURE    |
                    XCB_EVENT_MASK_STRUCTURE_NOTIFY;

    uint16_t width = 512;
    uint16_t height = 512;
    xcb_create_window(m_xcb_connection,
                      XCB_COPY_FROM_PARENT,
                      m_xcb_window,
                      m_xcb_screen->root,
                      0, 0, width, height,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      m_xcb_screen->root_visual,
                      value_mask,
                      value_list);

    // Magic code that will send notification when window is destroyed
    xcb_intern_atom_cookie_t  cookie  = xcb_intern_atom(m_xcb_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t  *reply   = xcb_intern_atom_reply(m_xcb_connection, cookie, 0);
    xcb_intern_atom_cookie_t  cookie2 = xcb_intern_atom(m_xcb_connection, 0, 16, "WM_DELETE_WINDOW");
    m_xcb_delete_window_wm_atom = xcb_intern_atom_reply(m_xcb_connection, cookie2, 0);

    xcb_change_property(m_xcb_connection, XCB_PROP_MODE_REPLACE, m_xcb_window, (*reply).atom, 4, 32, 1, &(*m_xcb_delete_window_wm_atom).atom);

    free(reply);

    xcb_map_window(m_xcb_connection, m_xcb_window);

    // Force the x/y coordinates to 100,100 results are identical in
    // consecutive
    // runs
    const uint32_t coords[] = { 100, 100 };
    xcb_configure_window(m_xcb_connection, m_xcb_window, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, coords);
}

void XCB_surface::create_xcb_surface(vk::Instance       vk_instance,
                                     vk::PhysicalDevice vk_physical_device)
{
    Expects(vk_instance);
    Expects(vk_physical_device);

    vk::XcbSurfaceCreateInfoKHR surface_create_info
    {
        vk::XcbSurfaceCreateFlagsKHR{},
        m_xcb_connection,
        m_xcb_window
    };

    m_vk_surface = vk_instance.createXcbSurfaceKHRUnique(surface_create_info);

    log_vulkan.trace("Created XCB surface\n");

    Surface::get_properties(vk_physical_device);

    Ensures(m_vk_surface);
}

void XCB_surface::xcb_handle_event(const xcb_generic_event_t *event)
{
    Expects(event != nullptr);

    uint8_t event_code = event->response_type & 0x7fu;
    switch (event_code)
    {
        case XCB_EXPOSE:
            // TODO: Resize window
            break;

        case XCB_CLIENT_MESSAGE:
        {
            auto *message = reinterpret_cast<const xcb_client_message_event_t *>(event);
            if (message->data.data32[0] == (*m_xcb_delete_window_wm_atom).atom)
            {
                m_quit = true;
            }
            break;
        }

        case XCB_KEY_RELEASE:
        {
            auto *key = reinterpret_cast<const xcb_key_release_event_t *>(event);

            switch (key->detail)
            {
                case 0x09u:  // Escape
                {
                    m_quit = true;
                    break;
                }

                case 0x71u:  // left arrow key
                {
                    break;
                }

                case 0x72u:  // right arrow key
                {
                    break;
                }

                case 0x41u:  // space bar
                {
                    m_pause = ~m_pause;
                    break;
                }
            }
            break;
        }

        case XCB_CONFIGURE_NOTIFY:
        {
            // TODO
            // auto *configure = reinterpret_cast<const xcb_configure_notify_event_t *>(event);
            // if ((width != configure->width) || (height != configure->height))
            // {
            //     width = configure->width;
            //     height = configure->height;
            //     resize();
            // }
            break;
        }

        default:
        {
            break;
        }
    }
}

void XCB_surface::xcb_run()
{
    xcb_flush(m_xcb_connection);

    while (!m_quit)
    {
        xcb_generic_event_t *event;

        if (m_pause)
        {
            event = xcb_wait_for_event(m_xcb_connection);
        }
        else
        {
            event = xcb_poll_for_event(m_xcb_connection);
        }

        while (event)
        {
            xcb_handle_event(event);
            free(event);
            event = xcb_poll_for_event(m_xcb_connection);
        }

        update();
    }
}

void XCB_surface::update()
{
}

} // namespace vipu
