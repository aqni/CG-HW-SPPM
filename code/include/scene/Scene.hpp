#pragma once
#ifndef __NR_SCENE_HPP__
#define __NR_SCENE_HPP__

#include "Texture.hpp"
#include "Material.hpp"
#include "Model.hpp"
#include "Light.hpp"
#include "Camera.hpp"

namespace NRenderer
{
    struct RenderOption
    {
        unsigned int width;
        unsigned int height;
        unsigned int depth;
        unsigned int samplesPerPixel;
        RenderOption()
            : width             (500)
            , height            (500)
            , depth             (4)
            , samplesPerPixel   (16)
        {}
    };

    struct Ambient
    {
        enum class Type
        {
            CONSTANT, ENVIROMENT_MAP
        };
        Type type;
        Vec3 constant = {};
        Handle environmentMap = {};
    };

    struct Scene
    {
        Camera camera;

        RenderOption renderOption;

        Ambient ambient;

        // buffers
        vector<Material> materials;
        vector<Texture> textures;

        vector<Model> models;
        vector<Node> nodes;
        // object buffer
        vector<Sphere> sphereBuffer;
        vector<Triangle> triangleBuffer;
        vector<Plane> planeBuffer;
        vector<Mesh> meshBuffer;
        vector<shared_ptr<Entity>> entityBuffer;
        vector<Light> lights;
        //第一个节点用来装sphere第二个用来装triangle
        vector<BVHNode>sceneTree;
        // light buffer
        vector<PointLight> pointLightBuffer;
        vector<AreaLight> areaLightBuffer;
        vector<DirectionalLight> directionalLightBuffer;
        vector<SpotLight> spotLightBuffer;
        void buildTree() {
            sceneTree.push_back(BVHNode(entityBuffer));
        }
    };
    using SharedScene = shared_ptr<Scene>;
} // namespace NRenderer


#endif