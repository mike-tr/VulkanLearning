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
    this->physicalDevice = VK_NULL_HANDLE;
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
            this->physicalDevice = d;
            cout << "Picked suitable device : " << deviceName << endl;
            break;
        }
    }

    if (this->physicalDevice == VK_NULL_HANDLE) {
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

void GEngine::createSwapChain() {
    vkWSIHelper::SwapChainSupportDetails swapChainSupport = vkWSIHelper::querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = vkWSIHelper::chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = vkWSIHelper::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = vkWSIHelper::chooseSwapExtent(window, swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t QueueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = QueueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error{"failed to create swap chain!"};
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    this->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    this->swapChainExtent = extent;
    this->swapChainImageFormat = surfaceFormat.format;
}

void GEngine::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.levelCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i])) {
            throw std::runtime_error{"failed to create image views!"};
        }
    }
}