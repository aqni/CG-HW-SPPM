#pragma once
#ifndef __GRID_HPP__
#define __GRID_HPP__

#include <forward_list>
#include <unordered_map>
#include <tuple>
#include <optional>

#include "VisablePoint.hpp"
#include "geometry/vec.hpp"

namespace sppm {
using namespace NRenderer;
using namespace std;
using SVP = SharedVisablePoint;

class Grid {
 public:
  Grid() = default;
  ~Grid() = default;

  void Index(const vector<SVP>& vps);
  const forward_list<SVP>* operator[](const Vec3& point) const;

 protected:
  void insert(const SVP& svp);
  tuple<glm::uvec3,bool> toGrid(const Vec3& point) const;
  size_t id(const glm::uvec3&) const;

 private:
  Vec3 max_point_{0};
  Vec3 min_point_{0};
  glm::uvec3 cell_nums_{0};
  double cell_width_{0};
  unordered_map<size_t, forward_list<SVP>> vp_lists_;
};

}  // namespace sppm

#endif  // __VISABLE_POINT_HPP__