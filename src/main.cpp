#include <cstdio>
#include <cstdlib>

#include "graphics/configuration.hpp"
#include "graphics/device.hpp"
#include "graphics/display.hpp"
#include "graphics/display_surface.hpp"
#include "graphics/instance.hpp"
#include "graphics/log.hpp"
#include "graphics/surface.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/xcb_surface.hpp"

using Configuration   = vipu::Configuration;
using Device          = vipu::Device;
using Display         = vipu::Display;
using Display_surface = vipu::Display_surface;
using Instance        = vipu::Instance;
using Surface         = vipu::Surface;
using Swapchain       = vipu::Swapchain;
using XCB_surface     = vipu::XCB_surface;

class Frame_in_flight
{
    vk::Semaphore image_acquired_ready_to_draw_semaphore;
    vk::Semaphore draw_complete_ready_to_present_semaphore;
};

// https://software.intel.com/content/www/us/en/develop/articles/practical-approach-to-vulkan-part-1.html

class Swapchain_entry
{
    vk::Image       image;
    vk::ImageView   image_view;
    vk::Framebuffer framebuffer;
};

class Vulkan
{
public:
    std::unique_ptr<Instance>  m_instance;
    std::unique_ptr<Surface>   m_surface;
    std::unique_ptr<Device>    m_device;
    std::unique_ptr<Swapchain> m_swapchain;
    //std::vector<Frame>        m_frames_in_flight;

    Vulkan()
    {
        Configuration configuration { Surface::Type::eXCB };

        m_instance = std::make_unique<Instance>(&configuration);

        auto &physical_device = m_instance->choose_physical_device();

        if (configuration.surface_type == Surface::Type::eDisplay)
        {
            vipu::log_vulkan.info("Creating display surface\n");

            m_surface = std::make_unique<Display_surface>(m_instance->get(),
                                                          &physical_device);
        }
        else if (configuration.surface_type == Surface::Type::eXCB)
        {
            vipu::log_vulkan.info("Creating XCB surface\n");

            m_surface = std::make_unique<XCB_surface>(m_instance->get(),
                                                      physical_device.get());
        }
        else
        {
            FATAL("invalid surface type\n");
        }

        m_device = std::make_unique<Device>(&physical_device, m_surface.get());

        m_swapchain = std::make_unique<Swapchain>(m_device.get(),
                                                  m_surface.get());

        m_swapchain.reset();
        m_device.reset();
        m_surface.reset();
        m_instance.reset();
    }

    ~Vulkan()
    {
        m_instance.reset();
    }
};

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    Vulkan vulkan;

    return 0;
}
