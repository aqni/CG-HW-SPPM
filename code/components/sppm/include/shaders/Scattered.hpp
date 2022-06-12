#pragma once
#ifndef __SCATTERED_HPP__
#define __SCATTERED_HPP__

#include "Ray.hpp"

namespace sppm
{
    struct Scattered
    {
        Ray ray = {};
        Vec3 attenuation = {};
        Vec3 emitted = {};
        bool isDiffuse = true;
        float pdf = {0.f};
    };
}

#endif