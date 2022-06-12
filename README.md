# 图形学大作业

## 程序使用说明

使用的渲染组件是SPPM。

可渲染的场景有：
- sppm.scn: 仅漫反射材质
- pt_glass.scn: 包含漫反射、绝缘体材质
- test_material.scn: 包含漫反射、金属

配置参数说明：
- depth: 传播深度
- samples: 渐进迭代次数


## 随机渐进光子映射算法(Stochastic Progressive Photon Mapping, SPPM)

一些在实现过程中学习的书籍和文章：

https://pbr-book.org/3ed-2018/Light_Transport_III_Bidirectional_Methods/Stochastic_Progressive_Photon_Mapping

https://blog.csdn.net/libing_zeng/article/details/76222498

https://zhuanlan.zhihu.com/p/259565623

若用户有设置单次迭代的光子数，就用这个数值；若用户没有设置单次迭代的光子数，就把像素点的个数作为单次迭代的光子数的默认值
