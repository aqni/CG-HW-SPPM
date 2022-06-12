#include "intersections/intersections.hpp"

namespace sppm::Intersection
{
    using namespace NRenderer;
    enum class Type
    {
        SPHERE = 0x0,
        TRIANGLE = 0X1,
        BVHNODE = 0x2
    };
    HitRecord xTriangle(const Ray& ray, const Triangle& t, float tMin, float tMax) {
        const auto& v1 = t.v1;
        const auto& v2 = t.v2;
        const auto& v3 = t.v3;
        const auto& normal = glm::normalize(t.normal);
        auto e1 = v2 - v1;
        auto e2 = v3 - v1;
        auto P = glm::cross(ray.direction, e2);
        float det = glm::dot(e1, P);
        Vec3 T;
        if (det > 0) T = ray.origin - v1;
        else { T = v1 - ray.origin; det = -det; }
        if (det < 0.000001f) return getMissRecord();
        float u, v, w;
        u = glm::dot(T, P);
        if (u > det || u < 0.f) return getMissRecord();
        Vec3 Q = glm::cross(T, e1);
        v = glm::dot(ray.direction, Q);
        if (v < 0.f || v + u > det) return getMissRecord();
        w = glm::dot(e2, Q);
        float invDet = 1.f / det;
        w *= invDet;
        if (w >= tMax || w <= tMin) return getMissRecord();
        return getHitRecord(w, ray.at(w), normal, t.material);

    }
    HitRecord xSphere(const Ray& ray, const Sphere& s, float tMin, float tMax) {
        const auto& position = s.position;
        const auto& r = s.radius;
        Vec3 oc = ray.origin - position;
        float a = glm::dot(ray.direction, ray.direction);
        float b = glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - r*r;
        float discriminant = b*b - a*c;
        if (discriminant <= 0)return getMissRecord();
        float sqrtDiscriminant = sqrt(discriminant);
        if (discriminant > 0) {
            float temp = (-b - sqrtDiscriminant) / a;
            if (temp < tMax && temp > tMin) {
                auto hitPoint = ray.at(temp);
                auto normal = (hitPoint - position)/r;
                return getHitRecord(temp, hitPoint, normal, s.material);
            }
            temp = (-b + sqrtDiscriminant) / a;
            if (temp < tMax && temp > tMin) {
                auto hitPoint = ray.at(temp);
                auto normal = (hitPoint - position)/r;
                return getHitRecord(temp, hitPoint, normal, s.material);
            }
        }
        return getMissRecord();
    }
    HitRecord xPlane(const Ray& ray, const Plane& p, float tMin, float tMax) {
        Vec3 normal = glm::normalize(p.normal);
        auto Np_dot_d = glm::dot(ray.direction, normal);
        if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f) return getMissRecord();
        float dp = -glm::dot(p.position, normal);
        float t = (-dp - glm::dot(normal, ray.origin))/Np_dot_d;
        if (t >= tMax || t <= tMin) return getMissRecord();
        // cross test
        Vec3 hitPoint = ray.at(t);
        Mat3x3 d{p.u, p.v, glm::cross(p.u, p.v)};
        d = glm::inverse(d);
        auto res  = d * (hitPoint - p.position);
        auto u = res.x, v = res.y;
        if ((u<=1 && u>=0) && (v<=1 && v>=0)) {
            return getHitRecord(t, hitPoint, normal, p.material);
        }
        return getMissRecord();
    }
    HitRecord xAreaLight(const Ray& ray, const AreaLight& a, float tMin, float tMax) {
        Vec3 normal = glm::cross(a.u, a.v);
        Vec3 position = a.position;
        auto Np_dot_d = glm::dot(ray.direction, normal);
        if (Np_dot_d < 0.0000001f && Np_dot_d > -0.00000001f) return getMissRecord();
        float dp = -glm::dot(position, normal);
        float t = (-dp - glm::dot(normal, ray.origin))/Np_dot_d;
        if (t >= tMax || t <= tMin) return getMissRecord();
        // cross test
        Vec3 hitPoint = ray.at(t);
        Mat3x3 d{a.u, a.v, glm::cross(a.u, a.v)};
        d = glm::inverse(d);
        auto res  = d * (hitPoint - position);
        auto u = res.x, v = res.y;
        if ((u<=1 && u>=0) && (v<=1 && v>=0)) {
            return getHitRecord(t, hitPoint, normal, {});
        }
        return getMissRecord();
    }
    bool xAABB(const Ray& ray, const AABB& box, float tMin, float tMax) {
        Vec3 inv_Direction = 1.0f / ray.direction;
        auto t0 = (box.min_corner - ray.origin) * inv_Direction;
        auto t1 = (box.max_corner - ray.origin) * inv_Direction;
        double temp;
        if (inv_Direction.x < 0) {
            temp = t0.x;
            t0.x = t1.x;
            t1.x = temp;
        }
        if (inv_Direction.y < 0) {
            temp = t0.y;
            t0.y = t1.y;
            t1.y = temp;
        }
        if (inv_Direction.z < 0) {
            temp = t0.z;
            t0.z = t1.z;
            t1.z = temp;
        }
        double minn = t0.x < t0.y ? t0.x : (t0.y < t0.z ? t0.y : t0.z);
        double maxx = t0.x > t0.y ? t0.x : (t0.y > t0.z ? t0.y : t0.z);
        tMin = minn < tMin ? minn : tMin;
        tMax = maxx < tMax ? maxx : tMax;
        return tMax > tMin;

    }
    HitRecord xBVHNode(const Ray& ray, const BVHNode& n, float tMin, float tMax){
             if (!xAABB(ray,n.bouding_box,tMin,tMax)) {
                return getMissRecord();
            }
            HitRecord hitLeft = nullopt;
            if (n.left->type ==Entity::Type::SPHERE) {
               auto sphere= (Sphere*)&(* n.left);
                hitLeft = xSphere(ray,*sphere, tMin, tMax);
            }
            else if (n.left->type == Entity::Type::TRIANGLE) {
                auto triangle = (Triangle*)&(*n.left);
                hitLeft = xTriangle(ray, *triangle, tMin, tMax);
            }
            else {
                auto bvh = (BVHNode*)&(*n.left);
                hitLeft = xBVHNode(ray, *bvh, tMin, tMax);
            }

            HitRecord hitRight = nullopt;
            if (n.right->type == Entity::Type::SPHERE) {
                auto sphere = (Sphere*)&(*n.right);
                hitRight = xSphere(ray, *sphere, tMin, hitLeft ? hitLeft->t : tMax);
            }
            else if (n.right->type == Entity::Type::TRIANGLE) {
                auto triangle = (Triangle*)&(*n.right);
                hitRight = xTriangle(ray, *triangle, tMin, hitLeft ? hitLeft->t : tMax);
            }
            else {
                auto bvh = (BVHNode*)&(*n.right);
                hitRight = xBVHNode(ray, *bvh, tMin,hitLeft ? hitLeft->t : tMax);
            }

            if(hitRight) return hitRight;
            return hitLeft;
    }
}