#pragma once

#include "VulkanHelper.h"
#include "SceneParser.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application
{
public:
    Application();
    ~Application();

    void loadScene(SceneStructure structure);
    void renderLoop();

private:
    GLFWwindow *window;
    bool framebufferResized = false;

    void initWindow();
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    VulkanHelper helper;
};
