#include "shaders/Lambertian.hpp"
#include "Onb.hpp"
#include "samplers/SamplerInstance.hpp"

namespace sppm {

Lambertian::Lambertian(Material& material, vector<Texture>& textures)
    : Shader(material, textures) {
  auto optDiffuseColor =
      material.getProperty<Property::Wrapper::RGBType>("diffuseColor");
  if (optDiffuseColor)
    diffuseColor = (*optDiffuseColor).value;
  else
    diffuseColor = {1, 1, 1};
}
RGB Lambertian::shade(const Vec3& in, const Vec3& out,
                      const Vec3& normal) const {
  return diffuseColor /PI;
}

Scattered Lambertian::shade(const Ray& ray, const Vec3& hitPoint,
                            const Vec3& normal) const {
  Vec3 origin = hitPoint;
  Vec3 random = defaultSamplerInstance<HemiSphere>().sample3d();

  Onb onb{normal};
  Vec3 direction = glm::normalize(onb.local(random));

  float pdf = 1 / (2 * PI);

  auto attenuation = shade(-ray.direction, direction, normal);

  return {Ray{origin, direction}, attenuation, Vec3{0},true, pdf};
}

}  // namespace sppm