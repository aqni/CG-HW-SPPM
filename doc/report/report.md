# �����������ӳ���㷨

����ֿ�(DDL֮����Ϊ����)�� https://gitee.com/AQgit/nrenderer

ʵ��ʵ�����ݣ�
- **��Ⱦ�㷨**�������������ӳ��(SPPM)
- **����**����������Ե��
- **����**��KD-Tree���Ż�

## �㷨����

### �����������ӳ���㷨

�����������ӳ���㷨[^sppm]��Stochastic Progressive Photon Mapping��SPPM����ÿ�ε��������¼������裺

1. ���������·���Ի�ÿ��ӵ㣨Visible Point����ÿ�����ػ��һ�����ӵ㣬ͬʱ�������ص�ֱ�ӹ��ա�
2. ʹ������Grid�������п��ӵ����������
3. ��Դ������͹��ӣ����ڳ����д��������ӱ��ۼӵ����еĿ��ӵ��У�
4. ͨ�����ӵ㲶��Ĺ��Ӽ������ص�**��ӹ���**���������Ĳ���뾶��

��ϸ�������Ƶ���ʵ��˼·�ο�������ԭ����[^sppm]�͡�PBRT�� [^pbrt]�����ﲻչ��˵�������Ƶ����ص��עʵ�ֹ����еļ������⡣

��һ�����������·��ʱ�������������ӳ���뽥������ӳ���㷨��ͬ��һ�����Ҫ��һ�����ƫ�ƣ����ܹ��ȽϺõ�����������ʵ�־����Ч����

�ڶ���ʹ������Grid����ø������ӵ��Ҫ���ݿ��ӵ���淨��͹������е���淨���ж��Ƿ���ͬһ�����ϡ�����Ӧֻ���Ǹ�ͬһ�����ϵĿ��ӵ��͹⣬����ᵼ�±��潻�紦���ȱ�����λ�øߣ�����������

��������Դ���͹��ӣ�Ӧ����ʵ�ִ�������͹��ߴ�������Ĳ�ͬ��

���ģ����������غ㡣���Ӵ���ʱ����Ӧ����˥�����������ӹ���ʱ����Ҫ���Թ������������������Խ�࣬����Խ����




### ��������
����������һ�ֽ��ƾ��淴��Ĳ��ʣ������䵽�������ʱ���������һ���������С��Χ�ķ��䣬�����������������������Է��䵽�ռ��ϵ��κ�λ�á�

�����뾵�淴����ʹ��ɵĽ����Ƿǳ������һ�ֲ��ʣ���Ϊ��������ȫ�����߰��շ��䶨�ɷ��䡣���м��㷴����ߵķ���ĺ������£�

```c++
static Vec3 reflect(const Vec3& normal, const Vec3& dir) {
	return dir - 2 * glm::dot(dir, normal) * normal;
}
```

���ݷ��䶨�ɣ�����ǵ�������ǣ����Ժ����ɵ�ͨ������������á�

����������Ҫ��������������shade������һ�����ڼ�����Ӧ����ɫֵ����һ�����ڼ�¼����ɢ�䷽��˥�������ʷֲ��ȵȲ�����

```c++
RGB Conductor::shade(const Vec3& in, const Vec3& out,
                     const Vec3& normal) const {
  return diffuseColor;
}

Scattered Conductor::shade(const Ray& ray, const Vec3& hitPoint,
                           const Vec3& normal) const {
  Vec3 origin = hitPoint;
  Vec3 direction = glm::reflect(ray.direction, normal);
  float pdf = 1;
  auto attenuation = diffuseColor;
  return {Ray{origin, direction}, attenuation, Vec3{0},false, pdf};
}
```

�����Ǿ��淴�䣬����ߵĸ��ʷֲ��Ǻ㶨��, ͬʱ���ǵ������������ɫ�����ڹ�������ʱ�����Ӧ��ɫ����˥������diffuseColor.

���Ǵ�����Ľ���������ȫ�Ǿ��淴��ģ������ᳯ��ĳһ�����ϼ��з��䣬���ǿ��������뷴�������һ����λ���������ͨ��fuzz_size���������Ʒ���ļ��г̶ȣ�

```c++
RGB Conductor::shade(const Vec3& in, const Vec3& out,
                     const Vec3& normal) const {
  Vec3 random = defaultSamplerInstance<UniformInSphere>().sample3d();
  Vec3 fuzz = Vec3(fuzz_size, fuzz_size, fuzz_size);
  Vec3 r = glm::normalize(reflect(normal, out) +fuzz * random);
  auto specular = diffuseColor * (1-glm::dot(glm::normalize(in), r));
  return specular;
}

Scattered Conductor::shade(const Ray& ray, const Vec3& hitPoint,
                           const Vec3& normal) const {
  Vec3 origin = hitPoint;
  Vec3 direction = glm::reflect(ray.direction, normal);

  float pdf = fuzz_size;

  auto attenuation = diffuseColor*Vec3(1-fuzz_size, 1 - fuzz_size, 1 - fuzz_size);

  return {Ray{origin, direction}, attenuation, Vec3{0},false, pdf};
}
```

�����뷴��Ļ������������������󣬿���ʹ�ý����������ģ�����������������Ч����

### ��Ե�����

��Ե������ǿ��Խ��й���͸��Ĳ��ʣ����粣����������ԣ���Ե����������������䶨�ɵĲ��ʣ������������������ߺͳ���������ڲ��ʵ������������й������䣬����������Ǵ���$90^{\circ}$��ʱ����ȫ��������

������Dielectric������ratio_index���������ʵ������ʣ�Ĭ�Ͽ�����������Ϊ1.��������������һ����1.5-1.6���ң��������ǻ���Ҫд������������߷���ĺ�����

```c++
static Vec3 refract(const Vec3& in, const  Vec3& normal, double refract_rate) {
    //���ݷ��������� ���б��� �������Ĺ��߷ֽ�Ϊ��ֱ���ַ����ƽ�з��߷���ֱ���㣬��������
    double cos_theta = clamp(dot(-in, normal), -1.0, 1.0);
    Vec3 Cos = Vec3(cos_theta, cos_theta, cos_theta);
    
    Vec3 out_perp = Vec3(refract_rate,refract_rate,refract_rate) * (in + Cos * normal);
    double temp = -sqrt(abs(1.0 - dot(out_perp, out_perp)));
    Vec3 out_para =Vec3(temp,temp,temp) * normal;
    Vec3 out = out_perp + out_para;
    return out;
}
```

���ǻ���Ҫ�����������ǣ���������������жϷ������仹��ȫ���䣺

```c++
Scattered Dielectric::shade(const Ray& ray, const Vec3& hitPoint,
        const Vec3& normal) const {
        auto attenuation = Vec3(1.0, 1.0, 1.0);
        //�жϹ����Ƿ��������ڲ�
        bool isOut = false;
        if (glm::dot(ray.direction, normal) < 0.0) {
            isOut = true;
        }
        float refract_ratio = isOut ? (1.0 / refract_index) : refract_index;
        Vec3 nownormal = isOut ? normal : -normal;
        //�ж��Ƿ���ȫ����
        float cos_theta = clamp(glm::dot(-glm::normalize(ray.direction), nownormal), 1.0, -1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        Vec3 outDirection;
        if (refract_ratio * sin_theta > 1.0  /* || reflectance(cos_theta, refract_ratio) > rand() / (RAND_MAX + 1.0)*/) {
            //ȫ����
            outDirection = glm::reflect(glm::normalize(ray.direction), nownormal);
        }
        else {
            //����
            outDirection = glm::refract(glm::normalize(ray.direction), nownormal, refract_ratio);
        }
        float pdf = 1.0;
        return { Ray{hitPoint, outDirection}, attenuation, Vec3{0}, pdf };
    }
```

���ˣ�������shadeCreator����Ӿ�Ե����ʵ�֧�ּ��ɡ�

### KD-Tree���Ż�

�ڽ���һ��Ĺ�����ʱ������������Ҫ�������е������ģ�ͣ��ֱ���й��������㣬��¼HitRecord�󷵻ؾ����������Ǹ����壬�������Ե�ʱ����ۻ�������ͼƬ��Ⱦ���ٶȡ�������ǿ��Կ���ʹ��KD-Tree���������Ż���

BVH(Bounding Volume Hierarchy Based On Tree)������Χ������һ��KD����������Ϊÿһ�����幹��AABB��Χ�С������������ռ��е��������ζ��֣�����BVH�ڵ㣬ÿһ���ڵ�İ�Χ���ǽڵ�����������������а�Χ�е�һ�ָ��ǡ�

����������Ҫ����AABB��Χ���ࣺ

```c++
struct AABB {
public:
    Vec3 min_corner, max_corner;
    AABB() {}
    AABB(Vec3 min, Vec3 max) :min_corner(min), max_corner(max) {}
    static AABB cover(AABB box1, AABB box2); //����������Χ�еıհ�  
};
```

ʹ�������Խ��߽ڵ���������Χ�У���������Ҫ����ÿһ������İ�Χ�С����￼�ǵ������ʹ����������ֻ���������������Ƭ�İ�Χ�й���:

```c++
//Sphere
virtual bool box(AABB& output_box) const  override{
            output_box = AABB(position - Vec3(radius, radius, radius), position + Vec3(radius, radius, radius));
            return true;
}
//Triangle
virtual bool box(AABB& output_box) const override {
    Vec3 min_corner = glm::min(glm::min(v1, v2), v3);
    Vec3 max_corner = glm::max(glm::max(v1, v2), v3);
    output_box = AABB(min_corner,max_corner);
    return true;
}
```

����������Ҫ����BVH�ڵ㣬�����ݽڵ㽨��:

```c++
struct  BVHNode : public Entity {
       public:
           shared_ptr<Entity> left;
           shared_ptr<Entity> right;
           AABB bouding_box;

           BVHNode();
           BVHNode(const vector<shared_ptr<Entity>>& list) : BVHNode(list, 0, list.size()) {};
           BVHNode(const std::vector<shared_ptr<Entity>>& src_objects, size_t start, size_t end) 		   {
               this->type = Type::BVHNODE;
               srand((int)time(NULL));
               auto objects = src_objects;//����һ�����ԸĶ���vector ���Ķ�ԭ����vector
               int axis = rand()%3;
               cout << "axis: " << axis << endl;
               //�ȽϺ����ĺ���ָ��
               auto compare = (axis == 0) ? compare_box_x :
                   ((axis == 1) ? compare_box_y :
                       compare_box_z);
               size_t object_num = end - start;
               cout << "start: " << start << " end: " << end << endl;
               //[start,end)����ҿ�
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
```

���������������б����ڵ�ʱ����������Ըýڵ�Ϊ���ڵ��BVH���Ĺ�����֮�����ǽ��������������ܼ��ɣ�

```c++
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
```

���ú󼴿�ʵ�ֹ����󽻼��١�

## ʵ����

### ��Ⱦ���������

|      ����1��       |        ����16��        |
| :------------------------: | :-----------------------------: |
| ![](img/diffuse_i1_t4.jpg) | ![](img/diffuse_i16_t21.jpg) |



### ��Ⱦ�����������Ե�����

| ��Ⱦ��������       | ��Ⱦ��Ե�����     |
| ------------------ | ------------------ |
| ![](img/metal.png) | ![](img/glass.png) |

### ��������

#### ������Ⱦ

����Ⱦ�����˲��л���ÿ�ε�����ʱ���£�

|       ����       | һ����ʱ/ms | ������ʱ/ms |
|:----------------:|------------:|------------:|
|     ����Ͷ��     |         220 |          40 |
|     �������     |         558 |         --- |
| ׷�ٹ���(iter=1) |        7441 |        2085 |
| ׷�ٹ���(iter=8) |        1538 |         439 |

���������Թ��������в��У��������л����ٿ�һ�㣬���ǲ���ʵ���ص㣬�������

#### KDTree���Ż�

| ����             | һ����ʱ/ms | KD-Tree��ʱ/ms |
| ---------------- | ----------- | -------------- |
| ����Ͷ��         | 220         | 201            |
| �������         | 558         | ---            |
| ׷�ٹ���(iter=1) | 7441        | 6826           |
| ׷�ٹ���(iter=8) | 1538        | 1487           |

����ʵ�ʲ����������������͸��ӶȲ����ߣ��������Ч��һ�㣬�����������϶�ʱ������Ч�����������.

## ʵ�����������������

### PPM��Ⱦ�����������

<img src="./img/grid_img.jpg" alt="PPM���BUG" style="width:50%;" />

�������ڱ�������������BUG����ش���(����)��i,j,k�߽�Ӧȡ"<="������ԭ���Ǹ��ֽ�㴦��`visable point`ǡ�ò��ܱ���Χ
������������������`git checkout grid_img`�� 

```cpp
// ����: sppm::Grid::insert
for (uint i = min_grid[0]; i < max_grid[0]; i++)
  for (uint j = min_grid[1]; j < max_grid[1]; j++)
    for (uint k = min_grid[2]; k < max_grid[2]; k++)
      vp_lists_[id({i, j, k})].push_front(svp);
```

### BackWall����

<img src="./img/backwall.jpg" alt="��ǽ����" style="width:50%;" />

���ָ������ԭ���ǣ�ֱ���հ���RayCast����е�Shader���������in��out��ָ�������ߴ������򡣶���PPM���ҽ�֮�����ߴ�������ʹ�ã����������ߴ�ֱ�ĺ�ǽ���������������`git checkout backwall`��


### �����ӹ��չ���

<img src="img/cornerbug.png" alt="�����ӹ��չ���" style="width:50%;" />

������ڹ�������������ϵĴ������������Ϊ���������Ϊ�£�����ԭ��������¡�����ӳ����������������ʵ�����ó�����cos(��)��б�ŵ�ƽ����ܵĹ���������ͶӰ��������ȣ���Ч������cosϵ������������Ҫ��cos(��)(��Ϊ������ӵ�ǿ��Ҫ����ͶӰ����Ĵ�С)����bug�Ĵ����`git checkout corner_bug`���޸ĺ�ļ�ӹ������¡�

## ʵ�����

ͨ������ʵ�֣��������������ϵ�ƫ�ͨ����bug�����۵ķ�����Ħ���������ӹ���ӳ���������������



[^sppm]: Hachisuka T, Jensen H W. Stochastic progressive photon mapping[M]//ACM SIGGRAPH Asia 2009 papers. 2009: 1-8.
[^pbrt]: [Stochastic Progressive Photon Mapping (pbr-book.org)](https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping)
