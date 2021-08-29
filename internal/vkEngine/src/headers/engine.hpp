#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

class GEngine {
public:
    GEngine(uint32_t width, uint32_t height);

    void run();

private:
    uint32_t width, height;
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSurfaceKHR surface;

    // Queues
    VkQueue graphicQueue;
    VkQueue presentQueue;

    static int kek;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void linkVulkan();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSurface();
};