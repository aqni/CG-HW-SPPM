#pragma once
#ifndef __DIELECTRIC_HPP__
#define __DIELECTRIC_HPP__

#include "Shader.hpp"

namespace sppm {
    class Dielectric : public Shader {
     private:
      float refract_index;

     public:
      Dielectric(Material& material, vector<Texture>& textures);

      RGB shade(const Vec3& in, const Vec3& out,
                        const Vec3& normal) const override;

      Scattered shade(const Ray& ray, const Vec3& hitPoint,
                      const Vec3& normal) const override;

      static double reflectance(float cos_theta, float refract_ratio) {
          double ratio = pow((1 - refract_ratio) / (1 + refract_ratio), 2);
          return ratio + (1 - ratio) * pow(1 - cos_theta, 5);
      }
    };
}  // namespace SimplePathTracer

#endif