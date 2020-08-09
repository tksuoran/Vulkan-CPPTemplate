

#include <cstdio>
#include <cstdlib>
#include <gsl/gsl>

#include "graphics/context.hpp"
#include "graphics/device.hpp"
#include "graphics/display.hpp"
#include "graphics/display_surface.hpp"
#include "graphics/instance.hpp"
#include "graphics/log.hpp"
#include "graphics/surface.hpp"
#include "graphics/swapchain.hpp"
#include "graphics/xcb_surface.hpp"
#include "graphics/vulkan.hpp"

using Context         = vipu::Context;
using Device          = vipu::Device;
using Display         = vipu::Display;
using Display_surface = vipu::Display_surface;
using Instance        = vipu::Instance;
using Surface         = vipu::Surface;
using Swapchain       = vipu::Swapchain;
using XCB_surface     = vipu::XCB_surface;

class Frame_in_flight
{
public:
    Frame_in_flight() = default;

    Frame_in_flight(Context &context)
    {
        Expects(context.vk_device);

        m_image_acquired_ready_to_draw_semaphore   = context.vk_device.createSemaphoreUnique( {} );
        m_draw_complete_ready_to_present_semaphore = context.vk_device.createSemaphoreUnique( {} );
        m_fence                                    = context.vk_device.createFenceUnique( {vk::FenceCreateFlagBits::eSignaled} );

        vk::CommandPoolCreateFlags command_pool_flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
                                                        vk::CommandPoolCreateFlagBits::eTransient;
        m_command_pool = context.vk_device.createCommandPoolUnique(
            {
                command_pool_flags,
                context.graphics_queue_family_index
            }
        );

        vk::CommandBufferAllocateInfo command_buffer_allocate_info(
            m_command_pool.get(),               // VkCommandPool        commandPool
            vk::CommandBufferLevel::ePrimary,   // VkCommandBufferLevel level
            1                                   // uint32_t             bufferCount
        );

        m_pre_command_buffer  = std::move(context.vk_device.allocateCommandBuffersUnique(command_buffer_allocate_info)[0]);
        m_post_command_buffer = std::move(context.vk_device.allocateCommandBuffersUnique(command_buffer_allocate_info)[0]);
    }

    void wait(Context &context)
    {
        vk::Fence  vk_fence  = m_fence.get();
        uint64_t   timeout_ns { 1000000000ULL }; // one second timeout
        vk::Bool32 wait_all   { VK_FALSE };

        auto result = context.vk_device.waitForFences(
            {
                vk_fence
            },
            wait_all,
            timeout_ns
        );
        VERIFY(result == vk::Result::eSuccess);

        context.vk_device.resetFences( { vk_fence } );
    }

    void acquire_image(Context &context)
    {
        uint64_t timeout_ns = 3000000000ULL; // 3 seconds
        uint32_t swapchain_image_index{std::numeric_limits<uint32_t>::max()};

        // Acquire swapchain image
        vk::Semaphore vk_semaphore = m_image_acquired_ready_to_draw_semaphore.get();
        auto res = context.vk_device.acquireNextImageKHR(
            context.vk_swapchain,
            timeout_ns,
            vk_semaphore,
            vk::Fence(),
            &swapchain_image_index
        );

        switch (res)
        {
            case vk::Result::eSuccess:
            {
                break;
            }

            case vk::Result::eSuboptimalKHR:
            {
                break;
            }

            case vk::Result::eErrorOutOfDateKHR:
            {
                // OnWindowSizeChanged();
                break;
            }

            default:
            {
                FATAL("acquireNextImageKHR failed.");
            }
        }
    }

    vk::UniqueFence          m_fence;
    vk::UniqueSemaphore      m_image_acquired_ready_to_draw_semaphore;
    vk::UniqueSemaphore      m_draw_complete_ready_to_present_semaphore;
    vk::UniqueFramebuffer    m_framebuffer;
    vk::UniqueCommandPool    m_command_pool;
    vk::UniqueCommandBuffer  m_pre_command_buffer;
    vk::UniqueCommandBuffer  m_post_command_buffer;
};

class Swapchain_entry
{
    vk::Image       image;
    vk::ImageView   image_view;
    vk::Framebuffer framebuffer;
};

class Vulkan
{
public:
    Context                       m_context;
    std::unique_ptr<Instance>     m_instance;
    std::unique_ptr<Surface>      m_surface;
    std::unique_ptr<Device>       m_device;

    std::unique_ptr<Swapchain>    m_swapchain;
    std::vector<Frame_in_flight>  m_frames_in_flight;
    size_t                        m_frame_resource_index{0};
    Frame_in_flight              *m_current_frame{nullptr};
    vk::UniqueRenderPass          m_renderpass;

    Vulkan()
    {
        m_context.surface_type = Surface::Type::eXCB;
        //m_context.surface_type = Surface::Type::eDisplay;

        m_instance = std::make_unique<Instance>(m_context);

        auto &physical_device = m_instance->choose_physical_device();
        m_context.physical_device = &physical_device;
        m_context.vk_physical_device = physical_device.get();

        if (m_context.surface_type == Surface::Type::eDisplay)
        {
            vipu::log_vulkan.info("Creating display surface\n");

            m_surface = std::make_unique<Display_surface>(m_context);
        }
        else if (m_context.surface_type == Surface::Type::eXCB)
        {
            vipu::log_vulkan.info("Creating XCB surface\n");

            m_surface = std::make_unique<XCB_surface>(m_context);
        }
        else
        {
            FATAL("invalid surface type\n");
        }

        m_context.surface    = m_surface.get();
        m_context.vk_surface = m_context.surface->get();

        m_device = std::make_unique<Device>(m_context);

        m_context.vk_device                   = m_device->get();
        m_context.vk_queue                    = m_device->get_queue();
        m_context.graphics_queue_family_index = m_device->get_queue_family_indices().graphics;

        m_swapchain = std::make_unique<Swapchain>(m_context);
        m_context.swapchain    = m_swapchain.get();
        m_context.vk_swapchain = m_context.swapchain->get();

        create_renderpasses();

        m_swapchain.reset();
        m_device.reset();
        m_surface.reset();
        m_instance.reset();
    }

    void create_renderpasses()
    {
        ///
        ///
        ///
        auto vk_format = m_context.swapchain->get_surface_format().format;

        vipu::log_vulkan.trace("attachment format {}", vk::to_string(vk_format));

        // Vulkan 1.0 + VK_KHR_create_renderpass2
        vk::AttachmentDescription2KHR color_attachment_description {
            vk::AttachmentDescriptionFlags{},
            vk_format,                              // B8G8R8A8Unorm
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp   ::eClear,
            vk::AttachmentStoreOp  ::eStore,
            vk::AttachmentLoadOp   ::eDontCare,  // stencil
            vk::AttachmentStoreOp  ::eDontCare,  // stencil
            vk::ImageLayout        ::eUndefined,
            vk::ImageLayout        ::ePresentSrcKHR
        };

        vk::AttachmentReference2KHR color_attachment_reference {
            0,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::ImageAspectFlagBits::eColor
        };

        vk::SubpassDescription2KHR subpass_description {
            vk::SubpassDescriptionFlags{},
            vk::PipelineBindPoint::eGraphics,
            0,                              // view mask
            0, nullptr,                     // input attachments
            1, &color_attachment_reference, // color attachments
            nullptr,                        // resolve attachments
            nullptr,                        // depth-stencil attachment
            0, nullptr                      // preserve attachments
        };

        vk::RenderPassCreateInfo2KHR render_pass_create_info {
            vk::RenderPassCreateFlags{},
            1, &color_attachment_description, // attachments
            1, &subpass_description,          // subpasses
            0, nullptr,                       // dependencies
            0, nullptr                        // correlated view masks
        };

        m_renderpass = m_context.vk_device.createRenderPass2KHRUnique(render_pass_create_info);

        m_renderpass.reset();
    }

    ~Vulkan()
    {
        m_instance.reset();
    }

    void begin_frame(Context &context)
    {
        ++context.frame_number;
        m_frame_resource_index = context.frame_number % m_frames_in_flight.size();

        m_current_frame = &m_frames_in_flight[m_frame_resource_index];
        m_current_frame->wait(context);
    }


    void end_frame()
    {
    }

};

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    Vulkan vulkan;

    return 0;
}
