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
        {
            if (sceneFile[current_index + 1] == 'E')
                return parseMesh();
            if (sceneFile[current_index + 1] == 'A')
                return parseMaterial();
        }
        if (token == 'C')
            return parseCamera();
        if (token == 'D')
            return parseDriver();
        if (token == 'E')
            return parseEnvironment();
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

        if (sceneFile[current_index + 1] != ',') // empty node
        {
            parseToLineEnd();
            object.object = node;
            return object;
        }
        else
        {
            parseToLineEnd();
        }
    }
    else if (sceneFile[current_index] == 'r')
    {
        throw std::logic_error("no translation values found!");
    }
    else if (sceneFile[current_index] == 's')
    {
        throw std::logic_error("no translation and rotation values found!");
    }
    else
    {
        current_index--; // no t&r&s
    }

    moveToken(1); // get first letter of the mesh/camera/environment/children
    bool onlyChildren = false;
    if (sceneFile[current_index] == 'm')
    {
        moveToken(6); // get first number of the mesh
        node.mesh = parseInteger();
    }
    else if (sceneFile[current_index] == 'c')
    {
        if (sceneFile[current_index + 1] == 'a')
        {
            moveToken(8); // get first number of the camera
            node.camera = parseInteger();
        }
        else if (sceneFile[current_index + 1] == 'h')
        {
            onlyChildren = true;
            moveToken(11); // get first number of the children
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
    }
    else if (sceneFile[current_index] == 'e')
    {
        moveToken(13); // get first number of the environment
        node.environment = parseInteger();
    }

    if (sceneFile[current_index] == ',' && !onlyChildren)
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

    MeshAttribute tangent;
    MeshAttribute texcoord;
    bool hasMaterial = false;
    moveToken(1); // get to tangent or color attribute
    if (sceneFile[current_index] == 'T')
    {
        hasMaterial = true;

        moveToken(18); // get first letter of the src
        tangent.src = parseString();
        parseToLineEnd();

        moveToken(9); // get first number of the offset
        tangent.offset = parseInteger();
        parseToLineEnd();

        moveToken(9); // get first number of the stride
        tangent.stride = parseInteger();
        parseToLineEnd();

        moveToken(10); // get first letter of the format
        tangent.format = parseString();
        parseToLineEnd();

        moveToken(20); // get first letter of the src
        texcoord.src = parseString();
        parseToLineEnd();

        moveToken(9); // get first number of the offset
        texcoord.offset = parseInteger();
        parseToLineEnd();

        moveToken(9); // get first number of the stride
        texcoord.stride = parseInteger();
        parseToLineEnd();

        moveToken(10); // get first letter of the format
        texcoord.format = parseString();
        parseToLineEnd();
        moveToken(1);
    }

    MeshAttribute color;
    moveToken(16); // get first letter of the src
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

    if (!hasMaterial)
    {
        current_index++; // skip '\"'
        getNextToken();  // finish parsing mesh

        mesh.attributes.push_back(position);
        mesh.attributes.push_back(normal);
        mesh.attributes.push_back(color);
    }
    else
    {
        parseToLineEnd();
        moveToken(11); // get first number of the material
        mesh.material = parseInteger();
        getNextToken(); // finish parsing mesh

        mesh.attributes.push_back(position);
        mesh.attributes.push_back(normal);
        mesh.attributes.push_back(tangent);
        mesh.attributes.push_back(texcoord);
        mesh.attributes.push_back(color);
    }

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

SceneObject SceneParser::parseMaterial()
{
    Material material;
    SceneObject object;
    std::vector<std::string> textures;

    object.type = Type::T_Material;
    material.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    material.name = parseString();
    parseToLineEnd();

    moveToken(1); // get first letter of next parameter
    if (sceneFile[current_index] == 'n')
    {
        current_index += 20; // get first letter of the normalMap src
        Texture texture;
        texture.src = parseString();
        textures.push_back(texture.src);
        if (sceneFile[current_index + 1] != ',')
        {
            material.normalMap = texture;
            parseToLineEnd();
            moveToken(1);
        }
        else
        {
            /* explicit type and format */
        }
    }
    else
    {
        Texture texture;
        texture.src = "default-normal.png";
        textures.push_back(texture.src);
        material.normalMap = texture;
    }
    if (sceneFile[current_index] == 'd')
    {
        current_index += 26; // get first letter of the displacementMap src
        Texture texture;
        texture.src = parseString();
        textures.push_back(texture.src);
        if (sceneFile[current_index + 1] != ',')
        {
            material.displacementMap = texture;
            parseToLineEnd();
            moveToken(1);
        }
        else
        {
            /* explicit type and format */
        }
    }
    if (sceneFile[current_index] == 'p')
    {
        current_index += 6; // get to albedo
        moveToken(9);       // get first letter of the albedo
        if (sceneFile[current_index] == '[')
        {
            // constant value
            moveToken(1); // get first number of the albedo
            int R = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();
            getNextToken();
            float G = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();
            getNextToken();
            float B = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();

            std::string filename = std::string("albedo") + std::to_string(object_index) + std::string(".png");
            createPNG(filename.c_str(), 1, 1, R, G, B, 255);
            Texture texture;
            texture.src = filename;
            textures.push_back(texture.src);
            material.pbr.emplace(); // must give it a empty value before call value()
            material.pbr.value().albedo = texture;
        }
        else
        {
            // texture
            moveToken(9); // get first letter of the albedo src
            Texture texture;
            texture.src = parseString();
            textures.push_back(texture.src);
            if (sceneFile[current_index + 1] != ',')
            {
                material.pbr.emplace(); // must give it a empty value before call value()
                material.pbr.value().albedo = texture;
                parseToLineEnd();
            }
            else
            {
                /* explicit type and format */
            }
        }

        moveToken(12); // get first number/letter of the roughness
        if (sceneFile[current_index] != '{')
        {
            int R = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();
            std::string filename = std::string("roughness") + std::to_string(object_index) + std::string(".png");
            createPNG(filename.c_str(), 1, 1, R, R, R, 255);
            Texture texture;
            texture.src = filename;
            textures.push_back(texture.src);
            material.pbr.value().roughness = texture;
        }
        else
        {
            moveToken(9); // get first letter of the roughness src
            Texture texture;
            texture.src = parseString();
            textures.push_back(texture.src);
            if (sceneFile[current_index + 1] != ',')
            {
                material.pbr.value().roughness = texture;
                parseToLineEnd();
            }
            else
            {
                /* explicit type and format */
            }
        }

        moveToken(12); // get first number/letter of the metalness
        if (sceneFile[current_index] != '{')
        {
            int R = static_cast<int>(parseFloat() * 255);
            parseToLineEnd(); // finish parsing material

            std::string filename = std::string("metalness") + std::to_string(object_index) + std::string(".png");
            createPNG(filename.c_str(), 1, 1, R, R, R, 255);
            Texture texture;
            texture.src = filename;
            textures.push_back(texture.src);
            material.pbr.value().metalness = texture;
        }
        else
        {
            moveToken(9); // get first letter of the metalness src
            Texture texture;
            texture.src = parseString();
            textures.push_back(texture.src);
            if (sceneFile[current_index + 1] != ',')
            {
                material.pbr.value().metalness = texture;
                parseToLineEnd(); // finish parsing material
            }
            else
            {
                /* explicit type and format */
            }
        }
    }
    else if (sceneFile[current_index] == 'l')
    {
        current_index += 14; // get to albedo
        moveToken(9);        // get first letter of the albedo
        if (sceneFile[current_index] == '[')
        {
            // constant value
            moveToken(1); // get first number of the albedo
            int R = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();
            getNextToken();
            float G = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();
            getNextToken();
            float B = static_cast<int>(parseFloat() * 255);
            parseToLineEnd();

            std::string filename = std::string("albedo") + std::to_string(object_index) + std::string(".png");
            createPNG(filename.c_str(), 1, 1, R, G, B, 255);
            Texture texture;
            texture.src = filename;
            textures.push_back(texture.src);
            material.lambertian.emplace(); // must give it a empty value before call value()
            material.lambertian.value().albedo = texture;
        }
        else
        {
            // texture
            moveToken(9); // get first letter of the albedo src
            Texture texture;
            texture.src = parseString();
            textures.push_back(texture.src);
            if (sceneFile[current_index + 1] != ',')
            {
                material.lambertian.emplace(); // must give it a empty value before call value()
                material.lambertian.value().albedo = texture;
                parseToLineEnd(); // finish parsing material
            }
            else
            {
                /* explicit type and format */
            }
        }
    }
    else if (sceneFile[current_index] == 'm')
    {
        material.mirror = true;
        parseToLineEnd(); // finish parsing material
    }
    else if (sceneFile[current_index] == 'e')
    {
        material.environment = true;
        parseToLineEnd(); // finish parsing material
    }
    else if (sceneFile[current_index] == 's')
    {
        material.simple = true;
        parseToLineEnd(); // finish parsing material
    }

    materialTexturePair.insert({material.id, textures});

    object.object = material;

    return object;
}

SceneObject SceneParser::parseEnvironment()
{
    Environment environment;
    SceneObject object;

    object.type = Type::T_Environment;
    environment.id = object_index;
    parseToLineEnd();

    moveToken(8); // get first letter of the name
    environment.name = parseString();
    parseToLineEnd();

    moveToken(20); // get first letter of the radiance src
    environment.radiance.src = parseString();
    parseToLineEnd();

    moveToken(8); // get first letter of the radiance type
    environment.radiance.type = parseString();
    parseToLineEnd();

    moveToken(10); // get first letter of the radiance format
    environment.radiance.format = parseString();
    parseToLineEnd(); // finish parsing environment

    object.object = environment;

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

    // record all texture file names that used in the scene for initScene
    sceneStructure.materialTexturePair = materialTexturePair;

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
        if (obj.type == Type::T_Environment)
        {
            sceneStructure.environment = std::get<Environment>(obj.object);
        }
        if (obj.type == Type::T_Material)
        {
            sceneStructure.materials.push_back(std::get<Material>(obj.object));
        }
    }

    for (auto root : sceneStructure.scene.roots)
    {
        std::vector<glm::mat4> parentTransforms;
        recordTransform(sceneStructure, std::get<Node>(sceneStructure.objects[root - 1].object), parentTransforms);
    }

    // record the relationship between meshes and materials
    if (!sceneStructure.materials.empty())
    {
        // for each mesh vbo, find the material index in the material list, for binding correct descriptor set
        for (auto mesh : sceneStructure.meshes)
        {
            uint32_t materialCount = 0;

            for (auto material : sceneStructure.materials)
            {
                if (mesh.mesh.material == material.id)
                {
                    sceneStructure.vboMaterialId.push_back(materialCount);
                    if (material.pbr.has_value())
                    {
                        sceneStructure.vboPipelineId.push_back(0);
                    }
                    else if (material.lambertian.has_value())
                    {
                        sceneStructure.vboPipelineId.push_back(1);
                    }
                    else if (material.mirror == true)
                    {
                        sceneStructure.vboPipelineId.push_back(2);
                    }
                    else if (material.environment == true)
                    {
                        sceneStructure.vboPipelineId.push_back(3);
                    }
                    break;
                }

                materialCount++;
            }
        }
    }

    return sceneStructure;
}

void SceneParser::recordTransform(SceneStructure &structure, Node node, std::vector<glm::mat4> parentTransforms, float time)
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
    glm::mat4 rotation = glm::mat4_cast(glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));

    if (time > 0.0f)
    {
        for (auto driver : structure.drivers)
        {
            if (node.id == driver.node)
            {
                if (driver.channel == "translation")
                {
                    glm::vec3 vec3;
                    getInterpolatedValue(vec3, driver.interpolation, driver, time);
                    translation = glm::translate(glm::mat4(1.0f), vec3);
                }
                else if (driver.channel == "rotation")
                {
                    glm::vec4 vec4;
                    getInterpolatedValue(vec4, driver.interpolation, driver, time);
                    rotation = glm::mat4_cast(glm::quat(vec4.w, vec4.x, vec4.y, vec4.z));
                }
                else if (driver.channel == "scale")
                {
                    glm::vec3 vec3;
                    getInterpolatedValue(vec3, driver.interpolation, driver, time);
                    scale = glm::scale(glm::mat4(1.0f), vec3);
                }
            }
        }
    }

    glm::mat4 transform = translation * rotation * scale;
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
    if (node.environment.has_value())
    {
        Environment environment = std::get<Environment>(structure.objects[node.environment.value() - 1].object);
        structure.environment.emplace(environment);
    }
    if (!node.children.empty())
    {
        for (auto child : node.children)
        {
            recordTransform(structure, std::get<Node>(structure.objects[child - 1].object), parentTransforms, time);
        }
    }
}

void SceneParser::getInterpolatedValue(glm::vec3 &vec, std::string method, const Driver &driver, float time)
{
    auto iter = std::lower_bound(driver.times.begin(), driver.times.end(), time);
    if (iter == driver.times.end())
    {
        vec = glm::vec3(*(driver.values.end() - 3), *(driver.values.end() - 2), *(driver.values.end() - 1));
        return;
    }

    uint32_t index = iter - driver.times.begin();
    if (method == "STEP")
    {
        vec = glm::vec3(driver.values[(index - 1) * 3], driver.values[(index - 1) * 3 + 1], driver.values[(index - 1) * 3 + 2]);
    }
    else if (method == "LINEAR")
    {
        float lerpValue = (time - *iter) / (*(iter - 1) - *iter);
        vec = glm::vec3(lerpValue * driver.values[(index - 1) * 3] + (1 - lerpValue) * driver.values[index * 3], lerpValue * driver.values[(index - 1) * 3 + 1] + (1 - lerpValue) * driver.values[index * 3 + 1], lerpValue * driver.values[(index - 1) * 3 + 2] + (1 - lerpValue) * driver.values[index * 3 + 2]);
    }
    else if (method == "SLERP")
    {
        // is this even possible?
    }
}

void SceneParser::getInterpolatedValue(glm::vec4 &vec, std::string method, const Driver &driver, float time)
{
    auto iter = std::lower_bound(driver.times.begin(), driver.times.end(), time);
    if (iter == driver.times.end())
    {
        vec = glm::vec4(*(driver.values.end() - 4), *(driver.values.end() - 3), *(driver.values.end() - 2), *(driver.values.end() - 1));
        return;
    }

    uint32_t index = iter - driver.times.begin();
    if (method == "STEP")
    {
        vec = glm::vec4(driver.values[(index - 1) * 4], driver.values[(index - 1) * 4 + 1], driver.values[(index - 1) * 4 + 2], driver.values[(index - 1) * 4 + 3]);
    }
    else if (method == "LINEAR")
    {
        float lerpValue = (time - *iter) / (*(iter - 1) - *iter);
        vec = glm::vec4(lerpValue * driver.values[(index - 1) * 4] + (1 - lerpValue) * driver.values[index * 4], lerpValue * driver.values[(index - 1) * 4 + 1] + (1 - lerpValue) * driver.values[index * 4 + 1], lerpValue * driver.values[(index - 1) * 4 + 2] + (1 - lerpValue) * driver.values[index * 4 + 2], lerpValue * driver.values[(index - 1) * 4 + 3] + (1 - lerpValue) * driver.values[index * 4 + 3]);
    }
    else if (method == "SLERP")
    {
        glm::vec4 q1 = glm::vec4(driver.values[(index - 1) * 4], driver.values[(index - 1) * 4 + 1], driver.values[(index - 1) * 4 + 2], driver.values[(index - 1) * 4 + 3]);
        glm::vec4 q2 = glm::vec4(driver.values[(index) * 4], driver.values[(index) * 4 + 1], driver.values[(index) * 4 + 2], driver.values[(index) * 4 + 3]);
        if (glm::dot(q1, q2) < 0.0f)
        {
            q2 = glm::vec4(-q2.x, -q2.y, -q2.z, -q2.w); // ensure interpolation is along the shortest path
        }
        float theta = std::acosf(glm::dot(q1, q2));
        float lerpValue = time - *(iter - 1);
        vec = std::sinf((1 - lerpValue) * theta) / std::sinf(theta) * q1 + std::sinf(lerpValue * theta) / std::sinf(theta) * q2;
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

void createPNG(const char *filename, int width, int height, int R, int G, int B, int A)
{
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
        std::cerr << "Error: unable to open file " << filename << " for writing." << std::endl;
        return;
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png)
    {
        std::cerr << "Error: unable to create PNG write structure." << std::endl;
        fclose(fp);
        return;
    }

    png_infop info = png_create_info_struct(png);
    if (!info)
    {
        std::cerr << "Error: unable to create PNG info structure." << std::endl;
        png_destroy_write_struct(&png, nullptr);
        fclose(fp);
        return;
    }

    if (setjmp(png_jmpbuf(png)))
    {
        std::cerr << "Error: an error occurred while creating PNG image." << std::endl;
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        return;
    }

    png_init_io(png, fp);

    // Set image attributes
    png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Write header
    png_write_info(png, info);

    // Allocate memory for image data
    png_bytep row = (png_bytep)malloc(4 * width * sizeof(png_byte));

    // Write image data
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            row[x * 4] = R;
            row[x * 4 + 1] = G;
            row[x * 4 + 2] = B;
            row[x * 4 + 3] = A;
        }
        png_write_row(png, row);
    }

    // Cleanup
    png_write_end(png, nullptr);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    free(row);
}
