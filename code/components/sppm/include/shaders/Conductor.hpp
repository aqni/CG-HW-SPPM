#pragma once
#ifndef __CONDUCTOR_HPP__
#define __CONDUCTOR_HPP__

#include "Shader.hpp"
#include "Scattered.hpp"
namespace sppm {
class Conductor : public Shader {
 private:
  Vec3 diffuseColor;
  double fuzz_size=0.1;
 public:
  Conductor(Material& material, vector<Texture>& textures);
  RGB shade(const Vec3& in, const Vec3& out,
                    const Vec3& normal) const override;
  Scattered shade(const Ray& ray, const Vec3& hitPoint,
                  const Vec3& normal) const override;
};
}  // namespace SimplePathTracer

#endif