#include "SceneParser.h"

SceneParser::SceneParser(const std::string &filename)
{
    sceneFile = readSceneFile(filename);
    max_index = static_cast<uint32_t>(sceneFile.size());
}

SceneParser::~SceneParser()
{
}

std::optional<SceneObject> SceneParser::parse()
{
    char token = getNextToken();
    if (token == '[')
    {
        current_index += 10; // skip "s72-v1",
        token = getNextToken();
    }
    if (token == '{')
    {
        object_index++;

        current_index += 1; // avoid extra space
        token = getNextToken();
        current_index += 8; // get first letter of the type
        token = getNextToken();

        if (token == 'S')
            return parseScene();
        if (token == 'N')
            return parseNode();
        if (token == 'M')
            return parseMesh();
        if (token == 'C')
            return parseCamera();
        if (token == 'D')
            return parseDriver();
    }
    if (token == ',')
    {
        moveToken(1);
        return std::nullopt;
    }
    else // token == ']'
    {
        finish = true;
        return std::nullopt;
    }
}

SceneObject SceneParser::parseScene()
{
    Scene scene;
    SceneObject object;

    object.type = Type::T_Scene;
    scene.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    scene.name = parseString();
    parseToLineEnd();

    moveToken(9); // get first number of the root
    while (sceneFile[current_index] != ']')
    {
        scene.roots.push_back(parseInteger());
        if (sceneFile[current_index] == ']')
            break;
        moveToken(1); // skip ','
    }
    current_index++; // skip ']'
    getNextToken();  // finish parsing scene

    object.object = scene;

    return object;
}

SceneObject SceneParser::parseNode()
{
    Node node;
    SceneObject object;

    object.type = Type::T_Node;
    node.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    node.name = parseString();
    parseToLineEnd();

    moveToken(1); // get first letter of the T/R/S
    if (sceneFile[current_index] == 't')
    {
        moveToken(14); // get first number of the translation
        node.translation[0] = parseFloat();
        parseToLineEnd();
        node.translation[1] = parseFloat();
        parseToLineEnd();
        node.translation[2] = parseFloat();
        parseToLineEnd();

        moveToken(12); // get first number of the rotation
        node.rotation[0] = parseFloat();
        parseToLineEnd();
        node.rotation[1] = parseFloat();
        parseToLineEnd();
        node.rotation[2] = parseFloat();
        parseToLineEnd();
        node.rotation[3] = parseFloat();
        parseToLineEnd();

        moveToken(9); // get first number of the scale
        node.scale[0] = parseFloat();
        parseToLineEnd();
        node.scale[1] = parseFloat();
        parseToLineEnd();
        node.scale[2] = parseFloat();
        parseToLineEnd();
    }
    else if (sceneFile[current_index] == 'r')
    {
        throw std::logic_error("no translation values found!");
    }
    else if (sceneFile[current_index] == 's')
    {
        throw std::logic_error("no translation and rotation values found!");
    }

    moveToken(1); // get first letter of the mesh/camera
    if (sceneFile[current_index] == 'm')
    {
        moveToken(6); // get first number of the mesh
        node.mesh = parseInteger();
    }
    else if (sceneFile[current_index] == 'c')
    {
        moveToken(8); // get first number of the camera
        node.camera = parseInteger();
    }
    else
    {
        throw std::logic_error("empty node!");
    }

    if (sceneFile[current_index] == ',')
    {
        parseToLineEnd();
        moveToken(12); // get first number of the children
        while (sceneFile[current_index] != ']')
        {
            node.children.push_back(parseInteger());
            if (sceneFile[current_index] == ']')
                break;
            moveToken(1); // skip ','
        }
        current_index++; // skip ']'
        getNextToken();  // finish parsing node
    }

    object.object = node;

    return object;
}

SceneObject SceneParser::parseMesh()
{
    Mesh mesh;
    SceneObject object;

    object.type = Type::T_Mesh;
    mesh.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    mesh.name = parseString();
    parseToLineEnd();

    moveToken(12); // get first letter of the topology
    mesh.topology = parseString();
    parseToLineEnd();

    moveToken(8); // get first number of the count
    mesh.count = parseInteger();
    parseToLineEnd();

    /* add indices later */

    MeshAttribute position;
    getNextToken();
    current_index += 14; // get to position attribute
    moveToken(20);       // get first letter of the src
    position.src = parseString();
    parseToLineEnd();

    moveToken(9); // get first number of the offset
    position.offset = parseInteger();
    parseToLineEnd();

    moveToken(9); // get first number of the stride
    position.stride = parseInteger();
    parseToLineEnd();

    moveToken(10); // get first letter of the format
    position.format = parseString();
    parseToLineEnd();

    MeshAttribute normal;
    moveToken(18); // get first letter of the src
    normal.src = parseString();
    parseToLineEnd();

    moveToken(9); // get first number of the offset
    normal.offset = parseInteger();
    parseToLineEnd();

    moveToken(9); // get first number of the stride
    normal.stride = parseInteger();
    parseToLineEnd();

    moveToken(10); // get first letter of the format
    normal.format = parseString();
    parseToLineEnd();

    MeshAttribute color;
    moveToken(17); // get first letter of the src
    color.src = parseString();
    parseToLineEnd();

    moveToken(9); // get first number of the offset
    color.offset = parseInteger();
    parseToLineEnd();

    moveToken(9); // get first number of the stride
    color.stride = parseInteger();
    parseToLineEnd();

    moveToken(10); // get first letter of the format
    color.format = parseString();
    current_index++; // skip '\"'
    getNextToken();  // finish parsing mesh

    mesh.attributes.push_back(position);
    mesh.attributes.push_back(normal);
    mesh.attributes.push_back(color);

    object.object = mesh;

    return object;
}

SceneObject SceneParser::parseCamera()
{
    Camera camera;
    SceneObject object;

    object.type = Type::T_Camera;
    camera.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    camera.name = parseString();
    parseToLineEnd();

    moveToken(1); // get first letter of the perspective
    if (sceneFile[current_index] == 'p')
    {
        current_index += 15; // get to aspect attribute
        moveToken(9);        // get first number of the aspect
        camera.perspective.aspect = parseFloat();
        parseToLineEnd();

        moveToken(7); // get first number of the vfov
        camera.perspective.vfov = parseFloat();
        parseToLineEnd();

        moveToken(7); // get first number of the near
        camera.perspective.near = parseFloat();

        if (sceneFile[current_index] == ',')
        {
            parseToLineEnd();
            moveToken(6); // get first number of the far
            camera.perspective.far = parseFloat();
        }
    }
    else
    {
        throw std::logic_error("require orthographic camera!");
    }

    // finish parsing camera

    object.object = camera;

    return object;
}

SceneObject SceneParser::parseDriver()
{
    Driver driver;
    SceneObject object;

    object.type = Type::T_Driver;
    driver.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    driver.name = parseString();
    parseToLineEnd();

    moveToken(7); // get first number of the node
    driver.node = parseInteger();
    parseToLineEnd();

    moveToken(11); // get first letter of the channel
    driver.channel = parseString();
    parseToLineEnd();

    moveToken(9); // get first number of the times
    while (sceneFile[current_index] != ']')
    {
        driver.times.push_back(parseFloat());
        if (sceneFile[current_index] == ']')
        {
            parseToLineEnd();
            break;
        }
        moveToken(1); // skip ','
    }

    moveToken(10); // get first number of the values
    while (sceneFile[current_index] != ']')
    {
        driver.values.push_back(parseFloat());
        if (sceneFile[current_index] == ']')
        {
            parseToLineEnd();
            break;
        }
        moveToken(1); // skip ','
    }

    /* assume always have interpolation */

    moveToken(17); // get first letter of the interpolation
    driver.interpolation = parseString();
    current_index++; // skip '\"'
    getNextToken();  // finish parsing driver

    object.object = driver;

    return object;
}

SceneStructure SceneParser::parseSceneStructure()
{
    SceneStructure sceneStructure;

    while (!finish)
    {
        std::optional<SceneObject> obj = parse();
        if (obj.has_value())
            sceneStructure.objects.push_back(obj.value());
    }

    for (auto obj : sceneStructure.objects)
    {
        if (obj.type == Type::T_Driver)
        {
            sceneStructure.drivers.push_back(std::get<Driver>(obj.object));
        }
        if (obj.type == Type::T_Scene)
        {
            sceneStructure.scene = std::get<Scene>(obj.object);
        }
    }

    for (auto root : sceneStructure.scene.roots)
    {
        std::vector<glm::mat4> parentTransforms;
        recordTransform(sceneStructure, std::get<Node>(sceneStructure.objects[root - 1].object), parentTransforms);
    }

    return sceneStructure;
}

void SceneParser::recordTransform(SceneStructure &structure, Node node, std::vector<glm::mat4> parentTransforms)
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2])) * glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2])) * glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    parentTransforms.push_back(transform);
    glm::mat4 totalTransform = glm::mat4(1.0f);
    for (size_t i = parentTransforms.size(); i > 0; --i)
    {
        totalTransform = parentTransforms[i - 1] * totalTransform;
    }

    if (node.mesh.has_value())
    {
        bool hasInstance = false;
        for (auto &renderInfo : structure.meshes)
        {
            if (node.mesh.value() == renderInfo.mesh.id)
            {
                hasInstance = true;
                renderInfo.transforms.push_back(totalTransform);
                break;
            }
        }

        if (!hasInstance)
        {
            MeshRenderInfo renderInfo;
            renderInfo.mesh = std::get<Mesh>(structure.objects[node.mesh.value() - 1].object);
            renderInfo.transforms.push_back(totalTransform);
            structure.meshes.push_back(renderInfo);
        }
    }
    if (node.camera.has_value())
    {
        CameraRenderInfo renderInfo;
        renderInfo.camera = std::get<Camera>(structure.objects[node.camera.value() - 1].object);
        renderInfo.transform = totalTransform;
        structure.cameras.push_back(renderInfo);
    }
    if (!node.children.empty())
    {
        for (auto child : node.children)
        {
            recordTransform(structure, std::get<Node>(structure.objects[child - 1].object), parentTransforms);
        }
    }
}

char SceneParser::getNextToken()
{
    while (std::isspace(sceneFile[current_index]) || sceneFile[current_index] == '}')
        current_index++;

    if (current_index >= max_index)
        throw std::logic_error("exceeded the file range!");

    return sceneFile[current_index];
}

void SceneParser::moveToken(uint32_t number)
{
    getNextToken();
    current_index += number;
    getNextToken();
}

void SceneParser::parseToLineEnd()
{
    while (sceneFile[current_index] != ',')
        current_index++;
    current_index++; // go to the next line
}

std::string SceneParser::parseString()
{
    std::string str = "";

    while (sceneFile[current_index] != '\"')
    {
        str += sceneFile[current_index];
        current_index++;
    }

    return str;
}

uint32_t SceneParser::parseInteger()
{
    std::string str = "";

    while (std::isdigit(sceneFile[current_index]))
    {
        str += sceneFile[current_index];
        current_index++;
    }

    return static_cast<uint32_t>(std::stoul(str));
}

float SceneParser::parseFloat()
{
    std::string str = "";

    while (std::isdigit(sceneFile[current_index]) || sceneFile[current_index] == '.' || sceneFile[current_index] == '-' || sceneFile[current_index] == 'e')
    {
        str += sceneFile[current_index];
        current_index++;
    }

    return std::stof(str);
}

bool SceneParser::finishParsing()
{
    return finish;
}

std::vector<char> readSceneFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to load .s72 file!");

    size_t fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
