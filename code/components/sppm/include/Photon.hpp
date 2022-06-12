#pragma once
#ifndef __PHOTON_HPP__
#define __PHOTON_HPP__

#include "Ray.hpp"

namespace sppm {
using namespace NRenderer;
using namespace std;

class Photon {
 public:
  Photon(const Ray& source, const RGB& color)
      : source_(source), color_(color) {}
  const Ray& source() const { return source_; }
  const RGB& color() const { return color_; }

  void set_source(const Ray& source) { source_ = source; }
  void set_color(const RGB& color) { color_ = color; }

 private:
  Ray source_;
  RGB color_;
};
}  // namespace sppm

#endif