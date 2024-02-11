#include "Application.h"

int main(int argc, char *argv[])
{
    std::string sceneFile = "";
    std::optional<std::string> camera;
    std::optional<std::string> device;
    uint32_t width = 800, height = 600;
    for (int i = 0; i < argc; ++i)
    {
        if (std::string(argv[i]) == "--scene")
        {
            sceneFile = argv[i + 1];
        }
        if (std::string(argv[i]) == "--camera")
        {
            camera = argv[i + 1];
        }
        if (std::string(argv[i]) == "--physical-device")
        {
            device = argv[i + 1];
        }
        if (std::string(argv[i]) == "--drawing-size")
        {
            width = static_cast<uint32_t>(std::stoul(argv[i + 1]));
            height = static_cast<uint32_t>(std::stoul(argv[i + 2]));
        }
    }

    try
    {
        Application app(width, height);
        SceneParser parser(sceneFile);
        SceneStructure sceneStructure = parser.parseSceneStructure();
        app.loadScene(sceneStructure);

        if (!camera.has_value())
        {
            camera = sceneStructure.cameras[0].camera.name;
        }
        app.renderLoop(sceneStructure, camera.value());
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}