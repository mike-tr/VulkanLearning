#include "headers/engine.hpp"
#include "headers/vkWSIHelpers.hpp"
#include <cstring>
#include <headers/vulkanValidation.hpp>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// default device extentions
const vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const VkSurfaceKHR &surface) {
    // Logic to find graphics queue family
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queue : queueFamilies) {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device, const vector<const char *> &deviceExtensions) {
    uint32_t extentionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extentionCount, nullptr);

    vector<VkExtensionProperties> availableExtentions{extentionCount};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extentionCount, availableExtentions.data());

    set<string> requiredExtensions{deviceExtensions.begin(), deviceExtensions.end()};
    for (const auto &extention : availableExtentions) {
        requiredExtensions.erase(extention.extensionName);
    }
    return requiredExtensions.empty();
}

tuple<bool, std::string> isDeviceSuitable(VkPhysicalDevice device, const VkSurfaceKHR &surface) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    bool isSuitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                      deviceFeatures.geometryShader;
    isSuitable &= findQueueFamilies(device, surface).isComplete();
    isSuitable &= checkDeviceExtensionSupport(device, deviceExtensions);

    bool swapChainAdequete = false;
    if (isSuitable) {
        vkWSIHelper::SwapChainSupportDetails swapChainSupport = vkWSIHelper::querySwapChainSupport(device, surface);
        swapChainAdequete = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    isSuitable &= swapChainAdequete;

    return {isSuitable, deviceProperties.deviceName};
}

void GEngine::pickPhysicalDevice() {
    physicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with vulkan support!");
    }
    vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &d : devices) {

        auto [suitable, deviceName] = isDeviceSuitable(d, surface);

        if (suitable) {
            physicalDevice = d;
            cout << "Picked suitable device : " << deviceName << endl;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void GEngine::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    // define the queue
    vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    set<uint32_t> uniqueQueueFamalies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    int i = 1;
    float queuePriority = 1.0f;
    for (auto queueFamily : uniqueQueueFamalies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    // set device features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // create info
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    // add layer validation
    if (vkValidate::enable) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(vkValidate::validationLayers.size());
        createInfo.ppEnabledLayerNames = vkValidate::validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error{"failed to create logical device!"};
    }
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    cout << "succsesfully created a logical device" << endl;
}

void GEngine::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error{"failed to create window surface!"};
    }
}