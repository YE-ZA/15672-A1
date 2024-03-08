#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <png.h>

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <optional>

enum class Type
{
    T_Scene,
    T_Node,
    T_Mesh,
    T_Camera,
    T_Driver,
    T_Material,
    T_Environment
};

struct Scene
{
    uint32_t id;
    std::string name;
    std::vector<uint32_t> roots;
};

struct Node
{
    uint32_t id;
    std::string name;
    std::vector<float> translation{0, 0, 0};
    std::vector<float> rotation{0, 0, 0, 1};
    std::vector<float> scale{1, 1, 1};
    std::vector<uint32_t> children{};
    std::optional<uint32_t> camera;
    std::optional<uint32_t> mesh;
    std::optional<uint32_t> environment;
};

struct MeshAttribute
{
    std::string src;
    uint32_t offset;
    uint32_t stride;
    std::string format;
};

struct Mesh
{
    uint32_t id;
    std::string name;
    std::string topology;
    uint32_t count;
    std::vector<MeshAttribute> attributes;
    std::optional<uint32_t> material;
};

struct CameraInfo
{
    float aspect;
    float vfov;
    float near;
    float far = FLT_MAX;
};

struct Camera
{
    uint32_t id;
    std::string name;
    // can be optional if support orthographic camera
    CameraInfo perspective;
};

struct Driver
{
    uint32_t id;
    std::string name;
    uint32_t node;
    std::string channel;
    std::vector<float> times;
    std::vector<float> values;
    std::string interpolation = "LINEAR";
};

struct Texture
{
    std::string src;
    std::string type = "2D";
    std::string format = "linear";
};

struct PBR
{
    Texture albedo;
    Texture roughness;
    Texture metalness;
};

struct Lambertian
{
    Texture albedo;
};

struct Material
{
    uint32_t id;
    std::string name;
    std::optional<Texture> normalMap;
    std::optional<Texture> displacementMap;
    std::optional<PBR> pbr;
    std::optional<Lambertian> lambertian;
    bool mirror = false;
    bool environment = false;
    bool simple = false;
};

struct Environment
{
    uint32_t id;
    std::string name;
    Texture radiance;
};

struct SceneObject
{
    std::variant<Scene, Node, Mesh, Camera, Driver, Material, Environment> object;
    Type type;
};

struct MeshRenderInfo
{
    Mesh mesh;
    std::vector<glm::mat4> transforms;
};

struct CameraRenderInfo
{
    Camera camera;
    glm::mat4 transform;
};

struct SceneStructure
{
    std::vector<MeshRenderInfo> meshes;
    std::vector<CameraRenderInfo> cameras;
    std::vector<Driver> drivers;
    Scene scene;
    std::optional<Environment> environment;
    std::vector<std::string> textures;
    std::vector<SceneObject> objects;
};

class SceneParser
{
private:
    std::vector<char> sceneFile;
    uint32_t current_index = 0;
    uint32_t max_index = 0;
    uint32_t object_index = 0;
    bool finish = false;
    std::vector<std::string> textures;

public:
    SceneParser(const std::string &filename);
    ~SceneParser();

    std::optional<SceneObject> parse();
    SceneObject parseScene();
    SceneObject parseNode();
    SceneObject parseMesh();
    SceneObject parseCamera();
    SceneObject parseDriver();
    SceneObject parseMaterial();
    SceneObject parseEnvironment();

    SceneStructure parseSceneStructure();
    static void recordTransform(SceneStructure &structure, Node node, std::vector<glm::mat4> parentTransforms, float time = 0.0f);
    static void getInterpolatedValue(glm::vec3 &vec, std::string method, const Driver &driver, float time);
    static void getInterpolatedValue(glm::vec4 &vec, std::string method, const Driver &driver, float time);

    char getNextToken();
    void moveToken(uint32_t number);
    void parseToLineEnd();
    std::string parseString();
    uint32_t parseInteger();
    float parseFloat();
    bool finishParsing();
};

static std::vector<char> readSceneFile(const std::string &filename);
static void createPNG(const char *filename, int width, int height, int R, int G, int B, int A);