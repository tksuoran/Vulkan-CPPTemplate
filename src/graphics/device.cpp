#include "fmt/format.h"
#include "gsl/gsl"

#include "graphics/device.hpp"
#include "graphics/context.hpp"
#include "graphics/log.hpp"
#include "graphics/physical_device.hpp"
#include "graphics/surface.hpp"
#include "graphics/vulkan.hpp"

namespace vipu
{

Device::Device(Context &context)
{
    Expects(context.physical_device != nullptr);
    Expects(context.surface != nullptr);

    Expects(context.vk_physical_device);

    m_queue_family_indices = context.physical_device->choose_queue_family_indices(context);

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

    // Fort GPU-Assisted validation:
    features.setFragmentStoresAndAtomics      (VK_TRUE);
    features.setVertexPipelineStoresAndAtomics(VK_TRUE);
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
    std::array<char const *, 2> device_extension_names = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
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

    m_vk_device = context.vk_physical_device.createDeviceUnique(device_create_info);

    vk::DeviceQueueInfo2 queue_info{
        vk::DeviceQueueCreateFlags(),
        m_queue_family_indices.graphics,
        0
    };

    VULKAN_HPP_DEFAULT_DISPATCHER.init(m_vk_device.get());

    m_vk_queue = m_vk_device->getQueue2(queue_info);

    log_vulkan.trace("{} completed\n", __func__);

    Ensures(m_vk_device);
    Ensures(m_vk_queue);
}

auto Device::get()
-> vk::Device
{
    return m_vk_device.get();
}

auto Device::get_queue_family_indices()
-> const Queue_family_indices &
{
    return m_queue_family_indices;
}

auto Device::get_queue()
-> vk::Queue
{
    return m_vk_queue;
}


} // namespace vipu
