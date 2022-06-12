#pragma once
#ifndef __INTERSECTIONS_HPP__
#define __INTERSECTIONS_HPP__

#include "HitRecord.hpp"
#include "Ray.hpp"
#include "scene/Scene.hpp"

namespace sppm
{
    namespace Intersection
    {
        HitRecord xTriangle(const Ray& ray, const Triangle& t, float tMin = 0.f, float tMax = FLOAT_INF);
        HitRecord xSphere(const Ray& ray, const Sphere& s, float tMin = 0.f, float tMax = FLOAT_INF);
        HitRecord xPlane(const Ray& ray, const Plane& p, float tMin = 0.f, float tMax = FLOAT_INF);
        HitRecord xAreaLight(const Ray& ray, const AreaLight& a, float tMin = 0.f, float tMax = FLOAT_INF);
        bool xAABB(const Ray& ray, const AABB& box, float tMin = 0.f, float tMax = FLOAT_INF);
       HitRecord xBVHNode(const Ray& ray, const BVHNode& n, float tMin = 0.f, float tMax = FLOAT_INF);
    }
}

#endif