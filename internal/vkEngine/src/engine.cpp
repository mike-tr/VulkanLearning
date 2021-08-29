#include "headers/engine.hpp"
#include <cstring>
#include <headers/vulkanValidation.hpp>
#include <vector>

using namespace std;

const bool logSupported = false;

GEngine::GEngine(uint32_t width, uint32_t height) : width(width), height(height) {
}

void GEngine::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void GEngine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    this->window = glfwCreateWindow(this->width, this->height, "Vulkan", nullptr, nullptr);
}

void GEngine::initVulkan() {
    this->linkVulkan();
    this->setupDebugMessenger();
    this->createSurface();
    this->pickPhysicalDevice();
    this->createLogicalDevice();
    this->createSwapChain();
    this->createImageViews();
}

void GEngine::linkVulkan() {
    if (vkValidate::enable && !vkValidate::checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "hello triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT ref{};
    vkValidate::addValidation(createInfo, ref);

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    vkValidate::checkRequiredAreSupportedExtensions();
}

void GEngine::setupDebugMessenger() {
    if (!vkValidate::enable) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    vkValidate::populateDebugMessengerCreateInfo(createInfo, "debug");

    if (vkValidate::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void GEngine::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void GEngine::cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);
    vkDestroyDevice(this->device, nullptr);
    if (vkValidate::enable) {
        vkValidate::DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(this->instance, surface, nullptr);
    vkDestroyInstance(this->instance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();
}