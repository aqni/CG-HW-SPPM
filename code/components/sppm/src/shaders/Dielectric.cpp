#include "shaders/Dielectric.hpp"
#include "Onb.hpp"
#include "samplers/SamplerInstance.hpp"
#include "samplers/UniformInSphere.hpp"
#include<cstdlib>
namespace sppm {

    static Vec3 reflect(const Vec3& normal, const Vec3& dir) {
        return dir - 2 * glm::dot(dir, normal) * normal;
    }

    static Vec3 refract(const Vec3& in, const  Vec3& normal, double refract_rate) {
        //根据菲涅尔定律 进行变形 将折射后的光线分解为垂直发现方向和平行法线方向分别计算，最后再相加
        double cos_theta = clamp(dot(-in, normal), -1.0, 1.0);
        Vec3 Cos = Vec3(cos_theta, cos_theta, cos_theta);
        
        Vec3 out_perp = Vec3(refract_rate,refract_rate,refract_rate) * (in + Cos * normal);
        double temp = -sqrt(abs(1.0 - dot(out_perp, out_perp)));
        Vec3 out_para =Vec3(temp,temp,temp) * normal;
        Vec3 out = out_perp + out_para;
        return out;
    }

    Dielectric::Dielectric(Material& material, vector<Texture>& textures)
        : Shader(material, textures) {
        auto optratio = material.getProperty<Property::Wrapper::FloatType>("ratio");
        if (optratio)
           refract_index = (*optratio).value;
        else
            refract_index = 1.5;
        cout << "refract_index: " << refract_index << endl;
    }

    RGB Dielectric::shade(const Vec3& in, const Vec3& out,
                         const Vec3& normal) const {
        return Vec3{ 0.0,0.0,0.0 };
    }

    Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint,
        const Vec3& normal) const {
        auto attenuation = Vec3(1.0, 1.0, 1.0);
        bool isOut = false;
        if (glm::dot(ray.direction, normal) < 0.0) {
            isOut = true;
        }
        float refract_ratio = isOut ? (1.0 / refract_index) : refract_index;
        Vec3 nownormal = isOut ? normal : -normal;
        //判断是否发生全反射
        float cos_theta = clamp(glm::dot(-glm::normalize(ray.direction), nownormal), 1.0, -1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        Vec3 outDirection;
        if (refract_ratio * sin_theta > 1.0  /* || reflectance(cos_theta, refract_ratio) > rand() / (RAND_MAX + 1.0)*/) {
            //全反射
            outDirection = glm::reflect(glm::normalize(ray.direction), nownormal);
        }
        else {
            //折射
           // cout << "refract" << endl;
            outDirection = glm::refract(glm::normalize(ray.direction), nownormal, refract_ratio);
        }
        float pdf = 1.0;
        return { Ray{hitPoint, outDirection}, attenuation, Vec3{0}, false, pdf };
    }

}  // namespace sppm