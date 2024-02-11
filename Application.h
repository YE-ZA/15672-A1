#pragma once

#include "VulkanHelper.h"
#include "SceneParser.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// camera
extern bool moveCamera;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;

extern bool firstMouse;
extern float yaw;
extern float pitch;
extern float lastX;
extern float lastY;
extern float fov;

class Application
{
public:
    Application(uint32_t width = WIDTH, uint32_t height = HEIGHT);
    ~Application();

    void loadScene(const SceneStructure &structure);
    void renderLoop(SceneStructure &structure, std::string &cameraName);

private:
    GLFWwindow *window;
    bool framebufferResized = false;
    VulkanHelper helper;

    bool pause = false;
    std::optional<std::chrono::steady_clock::time_point> startAnimTime;
    float lastPauseTime = 0.0f;
    float currentAnimTime = 0.0f;
    float maxAnimTime = 3.75f;
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    void initWindow(uint32_t width, uint32_t height);
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    static void mouseCallback(GLFWwindow *window, double xposIn, double yposIn);
    static void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    void updateTime();
    void updateScene(SceneStructure &structure, std::vector<glm::mat4> &uniformData, glm::mat4 &view, glm::mat4 &proj, std::string &cameraName);

    std::string switchCamera(const std::vector<CameraRenderInfo> &cameras, const std::string &cameraName);
    bool switchNextCamera = false;
    bool switchPrevCamera = false;

    void processInput(GLFWwindow *window);
    bool spaceKeyDown = false;
    bool leftKeyDown = false;
    bool rightKeyDown = false;
};
