#pragma once
#ifndef __VISABLE_POINT_HPP__
#define __VISABLE_POINT_HPP__

#include "Photon.hpp"
#include "geometry/vec.hpp"
#include "shaders/Shader.hpp"
#include <mutex>

namespace sppm {
using namespace NRenderer;
using namespace std;

class VisablePoint : public Vec3 {
 public:
  VisablePoint() = default;
  ~VisablePoint() = default;

  void set_direction(const Vec3& d) { reflected_direction_ = d; }
  void set_normal(const Vec3& n) { normal_ = n; }
  void set_attenuation(const RGB& att) { attenuation_ = att; }
  void add_attenuation(const RGB& att) { attenuation_ *= att; }
  void set_shader(const SharedShader& shader) { shader_ = shader; }
  void set_radius(double radius) { radius_ = radius; }
  double radius() const { return radius_; }
  bool cover(const Vec3& p) const {
    return glm::dot(p - *this, p - *this) <= radius_ * radius_;
  }
  void AddDirectLight(const RGB& color) { direct_lighting_ += color; }
  void Accumulate(const Photon& photon);
  void Update(double contribution = 0.7);
  RGB color(size_t iter_num, size_t photon_per_iter) const;
  Vec3 normal() const { return normal_; }
  SharedShader shader() const { return shader_; }

 private:
  Vec3 reflected_direction_{0};
  Vec3 normal_{0};
  SharedShader shader_ = nullptr;
  RGB direct_lighting_{0};
  RGB total_indirect_lighting_{0};
  RGB iter_indirect_lighting_{0};
  RGB attenuation_{1};
  double radius_ = 0;
  size_t total_photon_num_ = 0;
  size_t iter_photon_num_ = 0;
  mutex mtx_;
};
SHARE(VisablePoint);
}  // namespace sppm

#endif  // __VISABLE_POINT_HPP__