#include "fmt/format.h"
#include "gsl/gsl"

#include "graphics/log.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/device.hpp"
#include "graphics/physical_device.hpp"
#include "graphics/surface.hpp"

namespace vipu
{

Device::Device(Physical_device *physical_device,
               Surface         *surface)
:   m_physical_device   {physical_device}
,   m_vk_physical_device{m_physical_device->get()}
,   m_surface           {surface}
{
    Expects(m_physical_device != nullptr);
    Expects(m_vk_physical_device);
    Expects(m_surface != nullptr);

    m_queue_family_indices = physical_device->choose_queue_family_indices(m_surface);

    std::array<const float, 1> priorities {0.0f};

    vk::DeviceQueueCreateInfo device_queue_create_info{
        vk::DeviceQueueCreateFlags(),
        m_queue_family_indices.graphics,
        1,
        priorities.data()
    };

    vk::PhysicalDeviceFeatures features;
    features.setRobustBufferAccess                     (VK_FALSE);
    features.setFullDrawIndexUint32                    (VK_FALSE);
    features.setImageCubeArray                         (VK_FALSE);
    features.setIndependentBlend                       (VK_FALSE);
    features.setGeometryShader                         (VK_FALSE);
    features.setTessellationShader                     (VK_FALSE);
    features.setSampleRateShading                      (VK_FALSE);
    features.setDualSrcBlend                           (VK_FALSE);
    features.setLogicOp                                (VK_FALSE);
    features.setMultiDrawIndirect                      (VK_FALSE);
    features.setDrawIndirectFirstInstance              (VK_FALSE);
    features.setDepthClamp                             (VK_FALSE);
    features.setDepthBiasClamp                         (VK_FALSE);
    features.setFillModeNonSolid                       (VK_FALSE);
    features.setDepthBounds                            (VK_FALSE);
    features.setWideLines                              (VK_FALSE);
    features.setLargePoints                            (VK_FALSE);
    features.setAlphaToOne                             (VK_FALSE);
    features.setMultiViewport                          (VK_FALSE);
    features.setSamplerAnisotropy                      (VK_FALSE);
    features.setTextureCompressionETC2                 (VK_FALSE); // Not supported by NVIDIA
    features.setTextureCompressionASTC_LDR             (VK_FALSE); // Not supported by NVIDIA
    features.setTextureCompressionBC                   (VK_FALSE);
    features.setOcclusionQueryPrecise                  (VK_FALSE);
    // -    VkPhysicalDeviceCooperativeMatrixFeaturesNV
    // -    VkPhysicalDeviceCornerSampledImageFeaturesNV
    // -    VkPhysicalDeviceCoverageReductionModeFeaturesNV
    // -    VkPhysicalDeviceCustomBorderColorFeaturesEXT
    // - NV VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV
    // -    VkPhysicalDeviceDepthClipEnableFeaturesEXT
    // -    VkPhysicalDeviceDescriptorIndexingFeatures
    // -    VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV
    // -    VkPhysicalDeviceDiagnosticsConfigFeaturesNV
    // -    VkPhysicalDeviceExclusiveScissorFeaturesNV
    // -    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT
    // -    VkPhysicalDeviceFeatures2
    // -    VkPhysicalDeviceFragmentDensityMap2FeaturesEXT
    // -    VkPhysicalDeviceFragmentDensityMapFeaturesEXT
    // -    VkPhysicalDeviceFragmentShaderBarycentricFeaturesNV
    // -    VkPhysicalDeviceFragmentShaderInterlockFeaturesEXT
    // -    VkPhysicalDeviceHostQueryResetFeatures
    // -    VkPhysicalDeviceImagelessFramebufferFeatures
    // -    VkPhysicalDeviceIndexTypeUint8FeaturesEXT
    // -    VkPhysicalDeviceInlineUniformBlockFeaturesEXT
    // -    VkPhysicalDeviceLineRasterizationFeaturesEXT
    // -    VkPhysicalDeviceMemoryPriorityFeaturesEXT
    // -    VkPhysicalDeviceMeshShaderFeaturesNV
    // -    VkPhysicalDeviceMultiviewFeatures
    // -    VkPhysicalDevicePerformanceQueryFeaturesKHR
    // -    VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT
    // -    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR
    // -    VkPhysicalDevicePrivateDataFeaturesEXT
    // -    VkPhysicalDeviceProtectedMemoryFeatures
    // -    VkPhysicalDeviceRayTracingFeaturesKHR
    // -    VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV
    // -    VkPhysicalDeviceRobustness2FeaturesEXT
    // -    VkPhysicalDeviceSamplerYcbcrConversionFeatures
    // -    VkPhysicalDeviceScalarBlockLayoutFeatures
    // -    VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures
    // -    VkPhysicalDeviceShaderAtomicInt64Features
    // -    VkPhysicalDeviceShaderClockFeaturesKHR
    // -    VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT
    // -    VkPhysicalDeviceShaderDrawParametersFeatures
    // -    VkPhysicalDeviceShaderFloat16Int8Features
    // -    VkPhysicalDeviceShaderImageFootprintFeaturesNV
    // -    VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL
    // -    VkPhysicalDeviceShaderSMBuiltinsFeaturesNV
    // -    VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures
    // -    VkPhysicalDeviceShadingRateImageFeaturesNV
    // -    VkPhysicalDeviceSubgroupSizeControlFeaturesEXT
    // -    VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT
    // -    VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT
    // -    VkPhysicalDeviceTimelineSemaphoreFeatures
    // -    VkPhysicalDeviceTransformFeedbackFeaturesEXT
    // -    VkPhysicalDeviceUniformBufferStandardLayoutFeatures
    // -    VkPhysicalDeviceVariablePointersFeatures
    // -    VkPhysicalDeviceVertexAttributeDivisorFeaturesEXT
    // -    VkPhysicalDeviceVulkanMemoryModelFeatures
    // -    VkPhysicalDeviceYcbcrImageArraysFeaturesEXT
    std::array<char const *, 1> device_extension_names = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    std::array<char const *, 1> layer_names = {
        "VK_LAYER_KHRONOS_validation"
    };

    vk::DeviceCreateInfo device_create_info{
        vk::DeviceCreateFlags(),
        1,
        &device_queue_create_info,
        layer_names.size(),
        layer_names.data(),
        device_extension_names.size(),
        device_extension_names.data(),
        &features
    };

    m_vk_device = m_vk_physical_device.createDeviceUnique(device_create_info);

    vk::DeviceQueueInfo2 queue_info{
        vk::DeviceQueueCreateFlags(),
        m_queue_family_indices.graphics,
        0
    };

    m_vk_queue = m_vk_device->getQueue2(queue_info);

    create_swapchain();

    log_vulkan.trace("{} completed\n", __func__);

    Ensures(m_vk_device);
    Ensures(m_vk_queue);
}

void Device::create_swapchain()
{
    Expects(m_physical_device != nullptr);
    Expects(m_vk_device);
    Expects(m_surface != nullptr);

    // TODO syncronization
    // vk::SemaphoreCreateInfo semaphore_create_info{};

    auto vk_surface = m_surface->get();
    auto format = m_surface->choose_format();

    vk::SurfaceCapabilitiesKHR surface_capabilities = m_vk_physical_device.getSurfaceCapabilitiesKHR(vk_surface);
    log_vulkan.trace("minImageCount           : {}\n",      surface_capabilities.minImageCount);
    log_vulkan.trace("maxImageCount           : {}\n",      surface_capabilities.maxImageCount);
    log_vulkan.trace("currentExtent           : {} x {}\n", surface_capabilities.currentExtent.width, surface_capabilities.currentExtent.height);
    log_vulkan.trace("minExtent               : {} x {}\n", surface_capabilities.minImageExtent.width, surface_capabilities.minImageExtent.height);
    log_vulkan.trace("maxExtent               : {} x {}\n", surface_capabilities.maxImageExtent.width, surface_capabilities.maxImageExtent.height);
    log_vulkan.trace("maxImageArrayLayers     : {}\n",      surface_capabilities.maxImageArrayLayers);
    log_vulkan.trace("supportedTransforms     : {}\n",      vk::to_string(surface_capabilities.supportedTransforms));
    log_vulkan.trace("currentTransform        : {}\n",      vk::to_string(surface_capabilities.currentTransform));
    log_vulkan.trace("supportedCompositeAlpha : {}\n",      vk::to_string(surface_capabilities.supportedCompositeAlpha));
    log_vulkan.trace("supportedUsageFlags     : {}\n",      vk::to_string(surface_capabilities.supportedUsageFlags));

    std::vector<vk::PresentModeKHR> present_modes = m_vk_physical_device.getSurfacePresentModesKHR(vk_surface);
    for (auto present_mode : present_modes)
    {
        log_vulkan.trace("    present mode : {}\n", vk::to_string(present_mode));
    }

    std::array<uint32_t, 1> queue_family_indices {
        m_queue_family_indices.graphics
    };

    vk::SwapchainCreateInfoKHR swapchain_create_info{
        vk::SwapchainCreateFlagsKHR{},              // flags
        vk_surface,                                 // surface
        surface_capabilities.minImageCount,         // min image count
        format.format,                              // image format
        format.colorSpace,                          // image color space
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
    m_vk_swapchain = m_vk_device->createSwapchainKHRUnique(swapchain_create_info);
}

} // namespace vipu
