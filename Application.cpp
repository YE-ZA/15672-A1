#include "Application.h"

Application::Application()
{
    initWindow();
}

Application::~Application()
{
    helper.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::loadScene(SceneStructure structure)
{
    helper.initVulkan(window);

    std::vector<std::string> vertexData;
    std::vector<glm::mat4> uniformData;
    std::vector<uint32_t> counts;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> posOffsets;
    std::vector<uint32_t> normalOffsets;
    std::vector<uint32_t> colorOffsets;
    std::vector<std::string> posFormats;
    std::vector<std::string> normalFormats;
    std::vector<std::string> colorFormats;

    for (auto meshInfo : structure.meshes)
    {
        vertexData.push_back(meshInfo.mesh.attributes[0].src);
        counts.push_back(meshInfo.mesh.count);
        strides.push_back(meshInfo.mesh.attributes[0].stride);
        posOffsets.push_back(meshInfo.mesh.attributes[0].offset);
        normalOffsets.push_back(meshInfo.mesh.attributes[1].offset);
        colorOffsets.push_back(meshInfo.mesh.attributes[2].offset);
        posFormats.push_back(meshInfo.mesh.attributes[0].format);
        normalFormats.push_back(meshInfo.mesh.attributes[1].format);
        colorFormats.push_back(meshInfo.mesh.attributes[2].format);

        for (auto transform : meshInfo.transforms)
        {
            uniformData.push_back(transform);
        }
    }

    helper.initScene(vertexData, uniformData, counts, strides, posOffsets, normalOffsets, colorOffsets, posFormats, normalFormats, colorFormats);
}

void Application::renderLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        helper.drawFrame(window);
    }

    vkDeviceWaitIdle(helper.getDevice());
}

void Application::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Renderer", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Application::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}
