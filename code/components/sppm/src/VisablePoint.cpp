#include "VisablePoint.hpp"

#include <cassert>
#include <cmath>
#include <numbers>

namespace sppm {
using namespace std;

RGB sppm::VisablePoint::color(size_t iter_num, size_t photon_per_iter) const {
  const auto& tau = total_indirect_lighting_;
  const auto& N_p = static_cast<float>(photon_per_iter) * iter_num;
  double pi = numbers::pi;
  auto r = radius_;

  if (iter_num == 0 || photon_per_iter == 0 || r == 0) 
      return direct_lighting_;

  // 相关公式：https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping#x2-StochasticProgressivePhotonMapping

  auto L = tau / static_cast<float>(N_p * pi * r * r);
  return L + direct_lighting_ / static_cast<float>(iter_num);
}

void sppm::VisablePoint::Accumulate(const Photon& photon) {
  assert(shader_);
  if (!shader_) return;

  // 相关公式：https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping#eq:sppm-phi

  auto f = shader_->shade(photon.source().direction, this->reflected_direction_, normal_);
  
  lock_guard l(mtx_);
  iter_indirect_lighting_ += f * photon.color();
  iter_photon_num_++;
}

void sppm::VisablePoint::Update(double contribution) {
  double gamma = contribution;
  double N = total_photon_num_;
  double M = iter_photon_num_;
  double r = radius_;
  const auto& tau = total_indirect_lighting_;
  const auto& beta = attenuation_;
  const auto& PHI = iter_indirect_lighting_;

  assert(r != 0);
  if (r == 0 || M==0) return;

  // 相关公式：https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping#eq:sppm-update-rules

  auto N_ = N + gamma * M;
  auto r_ = r * sqrt(N_ / (N + M));
  auto tau_ = (tau + beta * PHI) * static_cast<float>((r_ * r_) / (r * r));

  total_photon_num_ = N_;
  iter_photon_num_ = 0;
  radius_ = r_;
  total_indirect_lighting_ = tau_;
  iter_indirect_lighting_ = RGB{0,0,0};
}
}  // namespace sppm
