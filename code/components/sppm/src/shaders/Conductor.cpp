#include "shaders/Conductor.hpp"

#include "Onb.hpp"
#include "samplers/SamplerInstance.hpp"
#include "samplers/UniformInSphere.hpp"

namespace sppm {

    static Vec3 reflect(const Vec3& normal, const Vec3& dir) {
        return dir - 2 * glm::dot(dir, normal) * normal;
    }
    Conductor::Conductor(Material& material, vector<Texture>& textures)
    : Shader(material, textures) {

     auto optDiffuseColor =
     material.getProperty<Property::Wrapper::RGBType>("reflect");
     if (optDiffuseColor)
        diffuseColor = (*optDiffuseColor).value;
    else
        diffuseColor = {1, 1, 1};
}
RGB Conductor::shade(const Vec3& in, const Vec3& out,
                     const Vec3& normal) const {
  Vec3 random = defaultSamplerInstance<UniformInSphere>().sample3d();
  Vec3 fuzz = Vec3(fuzz_size, fuzz_size, fuzz_size);
  Vec3 r = glm::normalize(reflect(normal, out) +fuzz * random);

  auto specular = diffuseColor * (1-glm::dot(glm::normalize(in), r));
  return Vec3(0,0,0);
}

Scattered Conductor::shade(const Ray& ray, const Vec3& hitPoint,
                           const Vec3& normal) const {
  Vec3 origin = hitPoint;
  Vec3 direction = glm::reflect(ray.direction, normal);

  float pdf = fuzz_size;

  auto attenuation = diffuseColor*Vec3(1-fuzz_size, 1 - fuzz_size, 1 - fuzz_size);

  return {Ray{origin, direction}, attenuation, Vec3{0},false, pdf};
}

}  // namespace sppm