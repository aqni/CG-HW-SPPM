#include "Grid.hpp"

#include <cmath>
#include <cstddef>
#include <execution>
#include <format>

#include "server/Server.hpp"

using namespace sppm;
using glm::uint;

void sppm::Grid::Index(const vector<SVP>& vps) {
  if (!vps.empty()) {
    max_point_ = min_point_ = *(vps[0]);
  }

  for (const auto& svp : vps) {
    if (svp->shader() == nullptr) continue;
    double r = svp->radius();
    auto max_point = *svp + static_cast<float>(r);
    auto min_point = *svp - static_cast<float>(r);
    max_point_ = glm::max(max_point_, max_point);
    min_point_ = glm::min(min_point_, min_point);
    cell_width_ = std::max(cell_width_, r);
  }

  assert(max_point_.x >= min_point_.x);
  assert(max_point_.y >= min_point_.y);
  assert(max_point_.z >= min_point_.z);

  cell_nums_ =
      glm::ceil((max_point_ - min_point_) / static_cast<float>(cell_width_));

  vp_lists_.clear();
  for_each(execution::seq, vps.begin(), vps.end(),
           [this](auto& svp) { insert(svp); });
}

void sppm::Grid::insert(const SVP& svp) {
  if (svp->shader() == nullptr) return;
  Vec3 max_point = *svp + static_cast<float>(svp->radius());
  Vec3 min_point = *svp - static_cast<float>(svp->radius());

  auto [max_grid, max_inbound] = toGrid(max_point);
  auto [min_grid, min_inbound] = toGrid(min_point);

  for (uint i = min_grid[0]; i <= max_grid[0]; i++) {
    for (uint j = min_grid[1]; j <= max_grid[1]; j++) {
      for (uint k = min_grid[2]; k <= max_grid[2]; k++) {
        vp_lists_[id({i, j, k})].push_front(svp);
      }
    }
  }
}

const forward_list<SVP>* sppm::Grid::operator[](const Vec3& point) const {
  auto [grid, inbound] = toGrid(point);
  auto grid_id = id(grid);
  if (inbound && vp_lists_.count(grid_id) > 0) {
    return &vp_lists_.at(grid_id);
  }
  return nullptr;
}

tuple<glm::uvec3, bool> sppm::Grid::toGrid(const Vec3& point) const {
  Vec3 offset = (point - min_point_) / (max_point_ - min_point_);
  glm::uvec3 grid = glm::ceil(offset * Vec3(cell_nums_));
  bool inbound = true;
  for (int i = 0; i < 3; ++i) {
    inbound &= grid[i] >= 0 && grid[i] < cell_nums_[i];
  }
  grid = clamp(grid, {0, 0, 0}, cell_nums_);
  return {grid, inbound};
}

size_t sppm::Grid::id(const glm::uvec3& c) const {
  size_t result = 0;
  result += c[0];
  result *= cell_nums_[1];
  result += c[1];
  result *= cell_nums_[2];
  result += c[2];
  return result;
}
