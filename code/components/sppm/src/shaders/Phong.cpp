#include "shaders/Phong.hpp"
#include "Onb.hpp"
#include "samplers/SamplerInstance.hpp"

namespace sppm
{
    static Vec3 reflect(const Vec3& normal, const Vec3& dir) {
        return dir - 2*glm::dot(dir, normal)*normal;
    }
    Phong::Phong(Material& material, vector<Texture>& textures)
        : Shader                (material, textures)
    {
        using PW = Property::Wrapper;
        auto optDiffuseColor = material.getProperty<PW::RGBType>("diffuseColor");
        if (optDiffuseColor) diffuseColor = (*optDiffuseColor).value;
        else diffuseColor = {1, 1, 1};
        
        auto optSpecularColor = material.getProperty<PW::RGBType>("specularColor");
        if (optSpecularColor) specularColor = (*optSpecularColor).value;
        else specularColor = {1, 1, 1};

        auto optSpecularEx = material.getProperty<PW::FloatType>("specularEx");
        if (optSpecularEx) specularEx = (*optSpecularEx).value;
        else specularEx = 1;

    }
    RGB Phong::shade(const Vec3& in, const Vec3& out, const Vec3& normal) const {
        Vec3 v = in;
        Vec3 r = reflect(normal, out);
        auto diffuse = diffuseColor * glm::dot(out, normal);
        auto specular = specularColor * fabs(glm::pow(glm::dot(v, r), specularEx));
        return diffuse + specular;
    }

    Scattered Phong::shade(const Ray& ray, const Vec3& hitPoint,
                                 const Vec3& normal) const {
      Vec3 origin = hitPoint;
      Vec3 random = defaultSamplerInstance<HemiSphere>().sample3d();

      Onb onb{normal};
      Vec3 direction = glm::normalize(onb.local(random));

      Vec3 r = reflect(normal, direction);

      float pdf = 1 / (2 * PI);

      auto attenuation = diffuseColor / PI;

      return {Ray{origin, direction}, attenuation, Vec3{0},true, pdf};
    }
    }