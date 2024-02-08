#include "Application.h"

int main()
{
    try
    {
        Application app;
        SceneParser parser("sg-Articulation.s72");
        SceneStructure sceneStructure = parser.parseSceneStructure();
        app.loadScene(sceneStructure);
        app.renderLoop();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}