#pragma once
#ifndef __SPP_RENDERER_HPP__
#define __SPP_RENDERER_HPP__

#include <memory>

#include "Camera.hpp"
#include "Grid.hpp"
#include "Photon.hpp"
#include "VisablePoint.hpp"
#include "intersections/intersections.hpp"
#include "samplers/Marsaglia.hpp"
#include "scene/Scene.hpp"
#include "shaders/ShaderCreator.hpp"
#include "PerfTest.hpp"

namespace sppm {

using namespace NRenderer;
using namespace std;
class SPPMRenderer {
 public:
  SPPMRenderer(SharedScene spScene, size_t photon_per_iter, size_t iternum,
               size_t max_depth, float init_radius, float camera_ran_scale,
               float disk_factor);
  ~SPPMRenderer() = default;

  void init();
  vector<RGBA> render();

 protected:
  void progress();
  void CastSight(const Ray& r, const SharedVisablePoint& svp,
                 int depth = 3) const;
  vector<Photon> ShootPhotons(size_t photon_num);
  void TracePhoton(const Photon& p);

  // 求交相关工具函数
  HitRecord HitObject(const Ray& r) const;
  tuple<float, Vec3> HitLightSource(const Ray& r) const;

 protected:
  SharedScene spScene;
  sppm::Camera camera;

  vector<SharedShader> shader_programs_;
  vector<SharedVisablePoint> viable_points_;
  Grid grids_;
  size_t iter_counter_;

  const size_t max_depth_;
  const size_t iter_num_;
  const size_t photon_per_iter_;
  const double init_radius_;
  const float camera_ran_scale_;
  const double disk_factor_;  // [0,1] 越靠近1越扁

  bool has_init_ = false;
};

}  // namespace sppm

#endif  //__SPP_RENDERER_HPP__