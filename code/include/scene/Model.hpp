#pragma once
#ifndef __NR_MODEL_HPP__
#define __NR_MODEL_HPP__

#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "geometry/vec.hpp"

#include "Material.hpp"
#include "common/macros.hpp"

namespace NRenderer
{
    using namespace std;
    struct AABB {
    public:
        Vec3 min_corner, max_corner;
        AABB() {}
        AABB(Vec3 min, Vec3 max) :min_corner(min), max_corner(max) {}
        static AABB cover(AABB box1, AABB box2); //计算两个包围盒的闭包  
    };
        
    struct Entity {
        enum class Type
        {
            SPHERE = 0x0,
            TRIANGLE = 0X1,
            BVHNODE = 0x2
        };
        virtual bool box(AABB& output_box) const {
            return false;
        };
        Handle material;
        public:
            Type type;
        
    };
    SHARE(Entity);

    struct Sphere : public Entity
    {
        Vec3 direction = {0, 0, 1};
        Vec3 position = {0, 0, 0};
        float radius = { 0 };
        Sphere() {
            type = Type::SPHERE;
        }
        virtual bool box(AABB& output_box) const  override{
            output_box = AABB(position - Vec3(radius, radius, radius), position + Vec3(radius, radius, radius));
            return true;
        }
    };
    SHARE(Sphere);
    
    struct Triangle : public Entity
    {
        union {
            struct {
                Vec3 v1;
                Vec3 v2;
                Vec3 v3;
            };
            Vec3 v[3];
        };
        Vec3 normal;
        Triangle()
            : v1            ()
            , v2            ()
            , v3            ()
            , normal         (0, 0, 1)
        {
            type=Type::TRIANGLE;
        }
        virtual bool box(AABB& output_box) const override {
            Vec3 min_corner = glm::min(glm::min(v1, v2), v3);
            Vec3 max_corner = glm::max(glm::max(v1, v2), v3);
            output_box = AABB(min_corner,max_corner);
            return true;
        }
    };
    SHARE(Triangle);

    struct Plane : public Entity
    {
        Vec3 normal = {0, 0, 1};
        Vec3 position = {};
        Vec3 u = {};
        Vec3 v = {};
        virtual bool box(AABB& output_box) const override {
            return false;
        }
    };
    SHARE(Plane);

    struct Mesh : public Entity
    {
        vector<Vec3> normals;
        vector<Vec3> positions;
        vector<Vec2> uvs;
        vector<Index> normalIndices;
        vector<Index> positionIndices;
        vector<Index> uvIndices;

        bool hasNormal() const {
            return normals.size() != 0;
        }

        bool hasUv() const {
            return uvs.size() != 0;
        }
        virtual bool box(AABB& output_box) const override {
           
            return false;
        }
    };
    SHARE(Mesh);

    struct Node
    {
        enum class Type
        {
            SPHERE = 0x0,
            TRIANGLE = 0X1,
            PLANE = 0X2,
            MESH = 0X3
        };
        Type type = Type::SPHERE;
        Index entity;
        Index model;
    };
    SHARE(Node);

    inline bool compare_box_x(const shared_ptr<Entity> hita, const shared_ptr<Entity> hitb)
    {
        AABB boxa, boxb;
        if (hita->box(boxa) && hitb->box(boxb)) {
            return boxa.min_corner.x < boxb.min_corner.x;
        }
        //cerr << "No box error!\n" ;
        return false;
    }
    inline bool compare_box_y(const shared_ptr<Entity> hita, const shared_ptr<Entity> hitb)
    {
        AABB boxa, boxb;
        if (hita->box(boxa) && hitb->box(boxb)) {
            return boxa.min_corner.y < boxb.min_corner.y;
        }
        //cerr << "No box error!\n";
        return false;
    }
    inline bool compare_box_z(const shared_ptr<Entity> hita, shared_ptr<Entity> hitb)
    {
        AABB boxa, boxb;
        if (hita->box(boxa) && hitb->box(boxb)) {
            return boxa.min_corner.z < boxb.min_corner.z;
        }
        //cerr << "No box error!\n";
        return false;
    }
   inline  AABB cover(AABB box1, AABB box2)
    {
        return AABB(min(box1.min_corner, box2.min_corner), max(box1.max_corner, box2.max_corner));
    }

    struct  BVHNode : public Entity {
        public:
            shared_ptr<Entity> left;
            shared_ptr<Entity> right;
            AABB bouding_box;

            BVHNode();
            BVHNode(const vector<shared_ptr<Entity>>& list) : BVHNode(list, 0, list.size()) {};
            BVHNode(const std::vector<shared_ptr<Entity>>& src_objects, size_t start, size_t end) {
                cout << "src_objects: " << src_objects.size() << endl;
                for (auto object : src_objects) {
                    cout  << "type: " << int(object->type) << endl;
                }
                this->type = Type::BVHNODE;
                srand((int)time(NULL));
                auto objects = src_objects;//创建一个可以改动的vector 不改动原本的vector
                int axis = rand()%3;
                cout << "axis: " << axis << endl;
                //比较函数的函数指针
                auto compare = (axis == 0) ? compare_box_x :
                    ((axis == 1) ? compare_box_y :
                        compare_box_z);
                size_t object_num = end - start;
                cout << "start: " << start << " end: " << end << endl;
                //[start,end)左闭右开
                switch (object_num) {
                case 1:
                    left = right = objects[start];
                    break;
                case 2:
                    left = objects[start];
                    right = objects[start + 1];
                    if (!compare(objects[start], objects[start + 1]))swap(left, right);
                    break;
                default:
                    sort(objects.begin() + start, objects.begin() + end, compare);
                    auto mid = start + object_num / 2;
                    left = make_shared<BVHNode>(objects, start, mid);
                    right =make_shared<BVHNode>(objects, mid, end);
                    break;
                }
                AABB leftBox, rightBox;
                if (left->box(leftBox) && right->box(rightBox)) {
                    cout <<"leftBox: "<<leftBox.min_corner << "  " << leftBox.max_corner << endl;
                    cout <<"rightBox: "<< rightBox.min_corner << "  " << rightBox.max_corner << endl;
                    bouding_box = cover(leftBox, rightBox);
                }
                else {
                    cerr << "No box error\n";
                }
            }
            virtual bool box(AABB& output_box) const override {
                output_box = bouding_box;
                return true;
            }
    };
    SHARE(BVHNode);
    struct Model {
        vector<Index> nodes;
        Vec3 translation = {0, 0, 0};
        Vec3 scale = {1, 1, 1};
    };
    SHARE(Model);
}

#endif