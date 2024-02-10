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

void Application::loadScene(const SceneStructure &structure)
{
    helper.initVulkan(window);

    std::vector<std::string> vertexData;
    size_t uboSize = 0;
    std::vector<uint32_t> counts;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> posOffsets;
    std::vector<uint32_t> normalOffsets;
    std::vector<uint32_t> colorOffsets;
    std::vector<std::string> posFormats;
    std::vector<std::string> normalFormats;
    std::vector<std::string> colorFormats;
    std::vector<uint32_t> instanceCounts;

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
        instanceCounts.push_back(meshInfo.transforms.size());

        for (auto transform : meshInfo.transforms)
        {
            uboSize++;
        }
    }

    helper.initScene(vertexData, uboSize, counts, strides, posOffsets, normalOffsets, colorOffsets, posFormats, normalFormats, colorFormats, instanceCounts);
}

void Application::renderLoop(SceneStructure &structure, const std::string &cameraName)
{
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glfwPollEvents();
        if (!pause)
        {
            updateTime();
        }
        else
        {
            lastPauseTime = time;
            startTime.reset();
        }

        std::vector<glm::mat4> uniformData;
        glm::mat4 view;
        glm::mat4 proj;
        updateScene(structure, uniformData, view, proj, cameraName);

        helper.drawFrame(window, uniformData, view, proj);
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

void Application::updateTime()
{
    if (!startTime.has_value())
    {
        startTime = std::chrono::high_resolution_clock::now();
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime.value()).count() + lastPauseTime;
    time = std::fmod(time, maxTime);
}

void Application::updateScene(SceneStructure &structure, std::vector<glm::mat4> &uniformData, glm::mat4 &view, glm::mat4 &proj, const std::string &cameraName)
{
    // update scene structure based on drivers
    structure.meshes.clear();
    structure.cameras.clear();
    for (auto root : structure.scene.roots)
    {
        std::vector<glm::mat4> parentTransforms;
        SceneParser::recordTransform(structure, std::get<Node>(structure.objects[root - 1].object), parentTransforms, time);
    }

    // record updated ubo
    for (auto meshInfo : structure.meshes)
    {
        for (auto transform : meshInfo.transforms)
        {
            uniformData.push_back(transform);
        }
    }

    bool findCamera = false;
    for (auto cameraInfo : structure.cameras)
    {
        if (cameraInfo.camera.name == cameraName)
        {
            findCamera = true;
            view = glm::inverse(glm::translate(glm::mat4(1.0f), cameraPos) * cameraInfo.transform); // need change
            proj = glm::perspective(cameraInfo.camera.perspective.vfov, cameraInfo.camera.perspective.aspect, cameraInfo.camera.perspective.near, cameraInfo.camera.perspective.far);
            proj[1][1] *= -1;
        }
    }

    if (!findCamera)
        throw std::runtime_error("camera doesn't exist!");
}

void Application::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyDown)
    {
        pause = !pause;
        spaceKeyDown = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        spaceKeyDown = false;

    // float cameraSpeed = static_cast<float>(1.0 / 240);
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //     cameraPos += glm::normalize(glm::cross(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0))) * cameraSpeed;
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //     cameraPos -= glm::normalize(glm::cross(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0))) * cameraSpeed;
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //     cameraPos -= cameraSpeed * glm::vec3(-1, 0, 0);
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //     cameraPos += cameraSpeed * glm::vec3(-1, 0, 0);
}
