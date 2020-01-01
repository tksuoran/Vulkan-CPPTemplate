#include <cstdio>
#include <cstdlib>
#include <vulkan/vulkan.h>

int main(int argc, const char **argv)
{
    static_cast<void>(argc);
    static_cast<void>(argv);

    VkApplicationInfo application_info = {};
    application_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pNext              = nullptr;
    application_info.pApplicationName   = "ApplicationName";
    application_info.applicationVersion = 1u;
    application_info.pEngineName        = "EngineName";
    application_info.engineVersion      = 1u;
    application_info.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext                   = nullptr;
    instance_create_info.flags                   = 0u;
    instance_create_info.pApplicationInfo        = &application_info;
    instance_create_info.enabledExtensionCount   = 0u;
    instance_create_info.ppEnabledExtensionNames = nullptr;
    instance_create_info.enabledLayerCount       = 0u;
    instance_create_info.ppEnabledLayerNames     = nullptr;

    VkInstance instance;
    VkResult res = vkCreateInstance(&instance_create_info, nullptr, &instance);
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER) {
        fprintf(stderr, "vkCreateInstance() VK_ERROR_INCOMPATIBLE_DRIVER\n");
        exit(-1);
    } else if (res != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance() error\n");
        exit(-1);
    }
    fprintf(stdout, "vkCreateInstance() success\n");

    vkDestroyInstance(instance, nullptr);
    return 0;
}
