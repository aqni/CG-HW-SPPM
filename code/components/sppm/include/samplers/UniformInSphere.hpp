#pragma once
#ifndef __UNIFORM_IN_SPHERE_HPP__
#define __UNIFORM_IN_SPHERE_HPP__

#include "Sampler3d.hpp"
#include <ctime>

namespace sppm
{
    using namespace std;
    class UniformInSphere : public Sampler3d
    {
    private:
        default_random_engine e;
        uniform_real_distribution<float> u;
    public:
        UniformInSphere()
            : e((unsigned int)time(0) + insideSeed())
            , u(-1, 1)
        {}
        Vec3 sample3d() override {
            float x{ 0 }, y{ 0 }, z{ 0 };
            do {
                x = u(e);
                y = u(e);
                z = u(e);
            } while ((x * x + y * y + z * z) > 1);
            return { x, y, z };
        }
    };
}

#endif