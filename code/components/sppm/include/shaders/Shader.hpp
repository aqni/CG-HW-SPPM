#pragma once
#ifndef __SHADER_HPP__
#define __SHADER_HPP__

#include "Scattered.hpp"
#include "common/macros.hpp"
#include "geometry/vec.hpp"
#include "scene/Scene.hpp"

namespace sppm {
using namespace NRenderer;
using namespace std;

constexpr float PI = 3.1415926535898f;

class Shader {
 protected:
  Material& material;
  vector<Texture>& textureBuffer;

 public:
  Shader(Material& material, vector<Texture>& textures)
      : material(material), textureBuffer(textures) {}

  // SPPM 此处 in out 按光线传播方向，而非实现传播方向
  virtual RGB shade(const Vec3& in, const Vec3& out,
                    const Vec3& normal) const = 0;
  virtual Scattered shade(const Ray& ray, const Vec3& hitPoint,
                          const Vec3& normal) const = 0;
};
SHARE(Shader);
}  // namespace sppm

#endif  // __SHADER_HPP__