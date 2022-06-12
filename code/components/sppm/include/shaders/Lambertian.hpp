#pragma once
#ifndef __LAMBERTIAN_HPP__
#define __LAMBERTIAN_HPP__

#include "Shader.hpp"

namespace sppm {
class Lambertian : public Shader {
 private:
  Vec3 diffuseColor;

 public:
  Lambertian(Material& material, vector<Texture>& textures);
  RGB shade(const Vec3& in, const Vec3& out,
                    const Vec3& normal) const override;
  Scattered shade(const Ray& ray, const Vec3& hitPoint,
                  const Vec3& normal) const override;
};
}  // namespace SimplePathTracer

#endif