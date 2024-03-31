#include "Application.h"

uint32_t WIDTH = 800;
uint32_t HEIGHT = 600;

bool moveCamera = false;
glm::vec3 cameraPos = glm::vec3(5.0f, 0.0f, 1.0f);
glm::vec3 cameraFront = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

bool firstMouse = true;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 0.0f;
float lastY = 0.0f;
float fov = 45.0f;

Application::Application(uint32_t width, uint32_t height)
{
    initWindow(width, height);
    WIDTH = width;
    HEIGHT = height;
}

Application::~Application()
{
    helper.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// implementation indicate that a scene can only have meshes with/without materials
void Application::loadScene(const SceneStructure &structure)
{
    helper.initVulkan(window);

    std::vector<std::string> vertexData;
    size_t uboSize = 0;
    std::vector<uint32_t> counts;
    std::vector<uint32_t> strides;
    std::vector<uint32_t> posOffsets;
    std::vector<uint32_t> normalOffsets;
    std::vector<uint32_t> tangentOffsets;
    std::vector<uint32_t> texcoordOffsets;
    std::vector<uint32_t> colorOffsets;
    std::vector<std::string> posFormats;
    std::vector<std::string> normalFormats;
    std::vector<std::string> tangentFormats;
    std::vector<std::string> texcoordFormats;
    std::vector<std::string> colorFormats;
    std::vector<uint32_t> instanceCounts;

    bool simpleMaterial = false;

    for (auto meshInfo : structure.meshes)
    {
        if (meshInfo.mesh.material.has_value())
        {
            vertexData.push_back(meshInfo.mesh.attributes[0].src);
            counts.push_back(meshInfo.mesh.count);
            strides.push_back(meshInfo.mesh.attributes[0].stride);
            posOffsets.push_back(meshInfo.mesh.attributes[0].offset);
            normalOffsets.push_back(meshInfo.mesh.attributes[1].offset);
            tangentOffsets.push_back(meshInfo.mesh.attributes[2].offset);
            texcoordOffsets.push_back(meshInfo.mesh.attributes[3].offset);
            colorOffsets.push_back(meshInfo.mesh.attributes[4].offset);
            posFormats.push_back(meshInfo.mesh.attributes[0].format);
            normalFormats.push_back(meshInfo.mesh.attributes[1].format);
            tangentFormats.push_back(meshInfo.mesh.attributes[2].format);
            texcoordFormats.push_back(meshInfo.mesh.attributes[3].format);
            colorFormats.push_back(meshInfo.mesh.attributes[4].format);
            instanceCounts.push_back(meshInfo.transforms.size());
        }
        else
        {
            simpleMaterial = true;

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
        }

        for (auto transform : meshInfo.transforms)
        {
            uboSize++;
        }
    }

    // separate record environment map texture
    std::string cubemap = "";
    if (structure.environment.has_value())
    {
        cubemap = structure.environment.value().radiance.src;
    }

    if (simpleMaterial)
    {
        helper.initScene(vertexData, uboSize, counts, strides, posOffsets, normalOffsets, colorOffsets, posFormats, normalFormats, colorFormats, instanceCounts, cubemap);
    }
    else
    {
        std::vector<uint32_t> materialId;
        for (auto material: structure.materials)
        {
            materialId.push_back(material.id);
        }
        helper.initScene(vertexData, uboSize, counts, strides, posOffsets, normalOffsets, tangentOffsets, texcoordOffsets, colorOffsets, posFormats, normalFormats, tangentFormats, texcoordFormats, colorFormats, instanceCounts, materialId, structure.vboMaterialId, structure.vboPipelineId, structure.materialTexturePair, cubemap);
    }
}

void Application::renderLoop(SceneStructure &structure, std::string &cameraName)
{
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glfwPollEvents();
        if (!pause)
        {
            updateTime();
        }
        else
        {
            lastPauseTime = currentAnimTime;
            startAnimTime.reset();
        }
        if (cameraName != "USER")
        {
            cameraName = switchCamera(structure.cameras, cameraName);
        }

        std::vector<glm::mat4> uniformData;
        glm::mat4 view;
        glm::mat4 proj;
        updateScene(structure, uniformData, view, proj, cameraName);

        helper.drawFrame(window, uniformData, view, proj, freezeRendering);
    }

    vkDeviceWaitIdle(helper.getDevice());
}

void Application::initWindow(uint32_t width, uint32_t height)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, "Vulkan Renderer", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
}

void Application::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
    WIDTH = width;
    HEIGHT = height;
}

// Reference from LearnOpenGL
void Application::mouseCallback(GLFWwindow *window, double xposIn, double yposIn)
{
    if (moveCamera)
    {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
        // adapt to blender coordinate
        cameraFront = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(cameraFront, 1.0f);
        cameraFront = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::vec4(cameraFront, 1.0f);
    }
}

void Application::updateTime()
{
    if (!startAnimTime.has_value())
    {
        startAnimTime = std::chrono::high_resolution_clock::now();
    }

    auto currentTime = std::chrono::high_resolution_clock::now();
    currentAnimTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startAnimTime.value()).count() + lastPauseTime;
    currentAnimTime = std::fmod(currentAnimTime, maxAnimTime);
}

void Application::updateScene(SceneStructure &structure, std::vector<glm::mat4> &uniformData, glm::mat4 &view, glm::mat4 &proj, std::string &cameraName)
{
    // update scene structure based on drivers
    structure.meshes.clear();
    structure.cameras.clear();
    for (auto root : structure.scene.roots)
    {
        std::vector<glm::mat4> parentTransforms;
        SceneParser::recordTransform(structure, std::get<Node>(structure.objects[root - 1].object), parentTransforms, currentAnimTime);
    }

    // record updated ubo
    for (auto meshInfo : structure.meshes)
    {
        for (auto transform : meshInfo.transforms)
        {
            uniformData.push_back(transform);
        }
    }

    if (cameraName != "USER")
    {
        bool findCamera = false;
        for (auto cameraInfo : structure.cameras)
        {
            if (cameraInfo.camera.name == cameraName)
            {
                findCamera = true;
                view = glm::inverse(cameraInfo.transform);
                proj = glm::perspective(cameraInfo.camera.perspective.vfov, cameraInfo.camera.perspective.aspect, cameraInfo.camera.perspective.near, cameraInfo.camera.perspective.far);
                proj[1][1] *= -1;

                float top = cameraInfo.camera.perspective.near * std::tanf(0.5f * cameraInfo.camera.perspective.vfov);
                helper.setCullingFrustum(cameraInfo.camera.perspective.aspect * top, top, -cameraInfo.camera.perspective.near, -cameraInfo.camera.perspective.far);
            }
        }

        if (!findCamera)
            throw std::runtime_error("camera doesn't exist!");
    }
    else
    {
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        proj = glm::perspective(glm::radians(fov), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 1000.0f);
        proj[1][1] *= -1;

        float top = 0.1f * std::tanf(0.5f * glm::radians(fov));
        helper.setCullingFrustum(static_cast<float>(WIDTH) / static_cast<float>(HEIGHT) * top, top, -0.1f, -1000.0f);
    }
}

void Application::scrollCallback(GLFWwindow *window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 10.0f)
        fov = 10.0f;
    if (fov > 90.0f)
        fov = 90.0f;
}

std::string Application::switchCamera(const std::vector<CameraRenderInfo> &cameras, const std::string &cameraName)
{
    auto iter = cameras.begin();
    for (iter; iter != cameras.end(); ++iter)
    {
        if ((*iter).camera.name == cameraName)
        {
            break;
        }
    }

    if (switchNextCamera)
    {
        switchNextCamera = false;
        if (std::next(iter) != cameras.end())
        {
            return (*std::next(iter)).camera.name;
        }
        else
        {
            return cameras.front().camera.name;
        }
    }
    if (switchPrevCamera)
    {
        switchPrevCamera = false;
        if (iter != cameras.begin())
        {
            return (*std::prev(iter)).camera.name;
        }
        else
        {
            return cameras.back().camera.name;
        }
    }

    return cameraName;
}

void Application::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !spaceKeyDown)
    {
        pause = !pause;
        spaceKeyDown = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        spaceKeyDown = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        moveCamera = true;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE)
    {
        moveCamera = false;
        firstMouse = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !rightKeyDown)
    {
        switchNextCamera = true;
        rightKeyDown = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE)
    {
        rightKeyDown = false;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && !leftKeyDown)
    {
        switchPrevCamera = true;
        leftKeyDown = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE)
    {
        leftKeyDown = false;
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !debugKeyDown)
    {
        freezeRendering = !freezeRendering;
        debugKeyDown = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE)
    {
        debugKeyDown = false;
    }

    float cameraSpeed = static_cast<float>(3.0f * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && moveCamera)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && moveCamera)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && moveCamera)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && moveCamera)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && moveCamera)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && moveCamera)
        cameraPos -= cameraSpeed * cameraUp;
}
