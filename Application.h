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
    VulkanHelper helper;

    bool pause = false;
    std::optional<std::chrono::steady_clock::time_point> startTime;
    float lastPauseTime = 0.0f;
    float time = 0.0f;
    float maxTime = 5.0f;

    void initWindow();
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void updateTime();
    void updateScene(SceneStructure &structure, std::vector<glm::mat4> &uniformData, glm::mat4 &view, glm::mat4 &proj, const std::string &cameraName);

    void processInput(GLFWwindow *window);
    bool spaceKeyDown = false;

    //========================
    glm::vec3 cameraPos = glm::vec3(0.0f);
};
