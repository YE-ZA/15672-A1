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

    void loadScene(const SceneStructure &structure);
    void renderLoop(SceneStructure &structure, const std::string &cameraName);

private:
    GLFWwindow *window;
    bool framebufferResized = false;

    void initWindow();
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void updateScene(SceneStructure &structure, std::vector<glm::mat4> &uniformData, glm::mat4 &view, glm::mat4 &proj, const std::string &cameraName);

    VulkanHelper helper;
};
