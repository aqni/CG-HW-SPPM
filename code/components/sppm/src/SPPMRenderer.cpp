#include "SPPMRenderer.hpp"

#include <cassert>
#include <execution>

#include "Onb.hpp"
#include "VertexTransformer.hpp"
#include "intersections/intersections.hpp"
#include "samplers/SamplerInstance.hpp"
#include "samplers/UniformInSquare.hpp"
#include "server/Server.hpp"

using namespace std;
using namespace sppm;

sppm::SPPMRenderer::SPPMRenderer(SharedScene spScene, size_t photon_per_iter,
                                 size_t iternum, size_t max_depth,
                                 float init_radius, float camera_ran_scale,
                                 float disk_factor)
    : spScene(spScene),
      camera(spScene->camera),
      iter_num_(iternum),
      init_radius_(init_radius),
      max_depth_(max_depth),
      disk_factor_(disk_factor),
      camera_ran_scale_(camera_ran_scale),
      photon_per_iter_(photon_per_iter),
      iter_counter_(0) {
  this->spScene->buildTree();
  assert(spScene);
  assert(init_radius > 0);
}

vector<RGBA> sppm::SPPMRenderer::render() {
  auto width = spScene->renderOption.width;
  auto height = spScene->renderOption.height;
  const size_t pixel_num = width * height;

  if (!has_init_) {
    // 转换全局坐标
    VertexTransformer vertexTransformer{};
    vertexTransformer.exec(spScene);
    // 构建kdtree
    this->spScene->buildTree();
    has_init_ = true;
  }

  // 构建着色器
  shader_programs_.clear();
  ShaderCreator shaderCreator{};
  for (auto& mtl : spScene->materials) {
    shader_programs_.push_back(shaderCreator.create(mtl, spScene->textures));
  }

  viable_points_.clear();
  viable_points_.resize(pixel_num);
  for (auto& svp : viable_points_) {
    svp = make_shared<VisablePoint>();
    svp->set_radius(init_radius_);
  }

  // 随机渐进迭代
  for (size_t i = 0; i < iter_num_; i++) {
    progress();
    iter_counter_++;
  }

  // 生成结果图
  vector<RGBA> pixels(pixel_num);
  for (size_t i = 0; i < pixel_num; i++) {
    RGB color = viable_points_[i]->color(iter_counter_, photon_per_iter_);
    pixels[i] = {color, 1};
  }

  return pixels;
}

void sppm::SPPMRenderer::progress() {
  // 并行：采样visable point并计算直接光照
  for_each(execution::par, viable_points_.begin(), viable_points_.end(),
           [this](auto& svp) {
             auto width = spScene->renderOption.width;
             auto height = spScene->renderOption.height;

             // 换算关系：auto& svp = viable_points_[(height - i - 1) * width +
             // j];
             int idx = std::distance(viable_points_.data(), &svp);
             int i = height - 1 - idx / width;
             int j = idx % width;

             auto ran = defaultSamplerInstance<UniformInCircle>().sample2d();
             float s = float(j) / width + camera_ran_scale_ * ran.x;
             float t = float(i) / height + camera_ran_scale_ * ran.y;
             auto ray = camera.shoot(s, t);

             CastSight(ray, svp, max_depth_);
           });

  {
    auto timer = GuardClock("index grid time:");
    grids_.Index(viable_points_);
  }

  auto photons = ShootPhotons(photon_per_iter_);

  {
    auto timer = GuardClock("trace photon time:");
    for_each(execution::par, photons.begin(), photons.end(),
             [this](auto& photon) { TracePhoton(photon); });
  }

  for (const auto& svp : viable_points_) {
    svp->Update();
  }
}

void sppm::SPPMRenderer::CastSight(const Ray& r, const SharedVisablePoint& svp,
                                   int depth) const {
  if (spScene->areaLightBuffer.size() < 1) return;
  auto& l = spScene->areaLightBuffer[0];
  auto r_in = r;
  svp->set_attenuation({1, 1, 1});
  for (int i = 0; i <= depth; i++) {
    auto hitObject = HitObject(r_in);
    auto [t, emitted] = HitLightSource(r_in);
    if (hitObject && hitObject->t < t) {  // 视线投向物体
      auto mtlHandle = hitObject->material;
      auto scattered = shader_programs_[mtlHandle.index()]->shade(
          r_in, hitObject->hitPoint, hitObject->normal);
      auto scatteredRay = scattered.ray;
      auto attenuation = scattered.attenuation;
      auto emitted = scattered.emitted;
      bool isDiffuse = scattered.isDiffuse;
      float n_dot_in = glm::dot(hitObject->normal, scatteredRay.direction);
      float pdf = scattered.pdf;

      /**
       * emitted      - Le(p, w_0)
       * next         - Li(p, w_i)
       * n_dot_in     - cos<n, w_i>
       * atteunation  - BRDF
       * pdf          - p(w)
       **/
      r_in = scatteredRay;
      svp->set_direction(-r_in.direction);  // 视线与光传播方向相反
      svp->Vec3::operator=(hitObject->hitPoint);
      svp->set_shader(shader_programs_[hitObject->material.index()]);
      svp->set_normal(hitObject->normal);
      svp->add_attenuation(attenuation);
      if (isDiffuse || i == depth) {
        auto uv_weight = defaultSamplerInstance<UniformInSquare>().sample2d();
        auto lpos = l.position + l.u * uv_weight[0] + l.v * uv_weight[1];
        auto s = glm::cross(l.u, l.v);
        auto lnormal = glm::normalize(s);
        auto in = hitObject->hitPoint - lpos;
        auto indir = glm::normalize(in);
        if (glm::dot(indir, s) > 0) {  // 面光源的反向不亮
          auto indistance = glm::length(in);
          auto shadowRay = Ray{hitObject->hitPoint, -indir};
          auto shadowHit = HitObject(shadowRay);
          if ((!shadowHit) ||
              (shadowHit && shadowHit->t > indistance)) {  //无遮挡
            // 计算直接光照
            auto k = shader_programs_[hitObject->material.index()]->shade(
                indir, -r.direction, hitObject->normal);
            float pdfpos = 1 / glm::length(s);
            RGB ld = l.radiance * glm::dot(lnormal, indir) / pdfpos;
            svp->AddDirectLight(k * ld / glm::dot(in, in));
          }
        }
        break;
      }
    } else if (t < FLOAT_INF) {  // 视线投向光源
      svp->set_shader(nullptr);
      // 计算直接光照
      svp->AddDirectLight(emitted);
      break;
    } else {  // 视线投向虚空
      svp->set_shader(nullptr);
      break;
    }
  }
}

vector<Photon> sppm::SPPMRenderer::ShootPhotons(size_t photon_num) {
  // 目前只考虑单一面光源
  vector<Photon> photons;
  if (spScene->areaLightBuffer.size() < 1) return photons;
  auto& l = spScene->areaLightBuffer[0];
  auto s = glm::cross(l.u, l.v);
  auto lnormal = glm::normalize(s);
  auto onb = Onb{lnormal};

  for (size_t i = 0; i < photon_num; i++) {
    // 面光源采样
    auto uv_weight = defaultSamplerInstance<UniformInSquare>().sample2d();
    auto lpos = l.position + l.u * uv_weight[0] + l.v * uv_weight[1];
    auto dir_ran = defaultSamplerInstance<HemiSphere>().sample3d();
    auto photon_dir = glm::normalize(onb.local(dir_ran));
    float costheta = glm::dot(lnormal, photon_dir);

    float pdfdir = 1 / (2 * PI);
    float pdfpos = 1 / (glm::length(s) * glm::dot(lnormal, photon_dir));
    auto I = 2.0f * PI * l.radiance / pdfdir /
             pdfpos;  // l.radiance 是单位立体角的辐射
    photons.emplace_back(Ray{lpos, photon_dir}, I);
  }

  return photons;
}

void sppm::SPPMRenderer::TracePhoton(const Photon& p) {
  Photon photon = p;

  for (size_t depth = 0; depth < max_depth_; depth++) {
    const auto& ray = photon.source();
    auto hit = HitObject(ray);
    if (!hit) break;

    const auto& hit_point = hit->hitPoint;
    const auto& hit_normal = hit->normal;
    const auto& hit_material = hit->material;

    // 不处理直接光照，向visable point进行累加
    if (depth > 0) {
      auto hit_visable_points = grids_[hit_point];
      if (hit_visable_points) {
        for (auto& svp : *hit_visable_points) {
          if (glm::dot(hit_normal, svp->normal()) >
              disk_factor_) {  // if 在圆盘上
            if (svp->cover(hit_point)) {
              svp->Accumulate(photon);
            }
          }
        }
      }
    }

    // 传播
    const auto& shader = shader_programs_[hit_material.index()];
    auto scattered = shader->shade(ray, hit_point, hit_normal);
    auto old_color = photon.color() + scattered.emitted;
    auto attenuation = scattered.attenuation *
                       abs(glm::dot(hit_normal, scattered.ray.direction)) /
                       scattered.pdf;
    auto new_color = attenuation * old_color;

    // 相关公式：https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping#eq:photon-scatter-weight

    constexpr Vec3 ew = {0.212671f, 0.715160f, 0.072169f};  // red green blue 频段能量占比
    auto new_color_y = glm::dot(ew, new_color);
    auto old_color_y = glm::dot(ew, old_color);
    float q = 1 - new_color_y / old_color_y;  // 光子被吸收，中止传播的概率
    q = max((float)0, q);

    auto ran = defaultSamplerInstance<UniformSampler>().sample1d();
    if (ran < q) break;

    photon.set_color(new_color / (1 - q));

    assert(photon.color()[0] >= 0);
    assert(photon.color()[1] >= 0);
    assert(photon.color()[2] >= 0);

    photon.set_source(scattered.ray);
  }
}

HitRecord sppm::SPPMRenderer::HitObject(const Ray& r) const {
  HitRecord closestHit = nullopt;
  float closest = FLOAT_INF;
  for (auto& s : spScene->sphereBuffer) {
    auto hitRecord = Intersection::xSphere(r, s, 0.1, closest);
    if (hitRecord && hitRecord->t < closest) {
      closest = hitRecord->t;
      closestHit = hitRecord;
    }
  }
  for (auto& t : spScene->triangleBuffer) {
    auto hitRecord = Intersection::xTriangle(r, t, 0.1, closest);
    if (hitRecord && hitRecord->t < closest) {
      closest = hitRecord->t;
      closestHit = hitRecord;
    }
  }
  // for (auto& n : spScene->sceneTree) {
  //     auto hitRecord = Intersection::xBVHNode(r, n, 0.000001, closest);
  //     if (hitRecord && hitRecord->t < closest) {
  //         closest = hitRecord->t;
  //         closestHit = hitRecord;
  //     }
  // }

  for (auto& p : spScene->planeBuffer) {
    auto hitRecord = Intersection::xPlane(r, p, 0.1, closest);
    if (hitRecord && hitRecord->t < closest) {
      closest = hitRecord->t;
      closestHit = hitRecord;
    }
  }

  return closestHit;
}

tuple<float, RGB> sppm::SPPMRenderer::HitLightSource(const Ray& r) const {
  // 目前只支持面光源
  RGB v = {};
  HitRecord closest = getHitRecord(FLOAT_INF, {}, {}, {});
  for (auto& a : spScene->areaLightBuffer) {
    auto hitRecord = Intersection::xAreaLight(r, a, 0.000001, closest->t);
    if (hitRecord && closest->t > hitRecord->t) {
      closest = hitRecord;
      v = a.radiance;
    }
  }
  return {closest->t, v};
}