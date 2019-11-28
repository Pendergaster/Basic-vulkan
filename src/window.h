/************************************************************
* Check license.txt in project root for license information *
*********************************************************** */

#ifndef WINDOW_H
#define WINDOW_H

#include "utils.h"

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#if defined(WINDOWS_PLATFORM)
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <stdio.h>

#define SCREENWIDTH 1000
#define SCREENHEIGHT 800

static GLFWwindow* g_window = NULL;

static void error_callback(int e, const char *d) {
    ABORT("GLFW error %d: %s\n", e, d);
}

void window_init() {
    glfwInit();
    // start glfw with out opengl context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwSwapInterval(0);

    g_window = glfwCreateWindow(SCREENWIDTH,SCREENHEIGHT, "vulkan app", NULL, NULL);
    glfwSetWindowUserPointer(g_window,NULL);
    //glfwSetFramebufferSizeCallback(g_window,framebuffer_resize_callback);
    glfwSetErrorCallback(error_callback);
    ASSERT_MESSAGE(g_window,"FAILED TO INIT WINDOW"); // failed to create window
}

void window_create_surface(const VkInstance instance,VkSurfaceKHR* surface) {
    if (glfwCreateWindowSurface(instance, g_window, NULL, surface) != VK_SUCCESS) {
        ABORT("failed to create window surface");
    }
}

static const char** window_get_required_extensions(u32* count) {
    return glfwGetRequiredInstanceExtensions(count);
}

void dispose_window() {
    glfwDestroyWindow(g_window);
    glfwTerminate();
}

#endif // WINDOW_H
