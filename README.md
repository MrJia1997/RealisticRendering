# 简单场景的真实感显示与漫游

---

1. 场景构建
  - 构建一个简单场景，含三个以上物体模型(网格文件.OBJ等) 
  - 场景文件格式自定义
2. 用Phong模型绘制场景
  - 投影模式：可切换正投影/透视投影
  - 纹理映射：图像纹理/几何纹理
  - 简单阴影生成
3. 场景漫游
  - 通过键盘操作移动相机位置
  - 碰撞检测(用物体的AABB进行)
4. 用光线跟踪算法渲染
  - 基本的光线跟踪
  - 物体表面的光照参数可修改
  - 尝试不同类型(点光源、平行光)、不同位置、不同颜色的光源

注意：附简单的交互操作说明

---

Qt 5.11.2: https://www.qt.io/

Trimesh2: https://gfx.cs.princeton.edu/proj/trimesh2/

CGAL 4.12: https://www.cgal.org/

Trent Reed's Tutorial of Qt5 w/ OpenGL: http://www.trentreed.net/topics/opengl/

懂deeee珍惜's Tutorial of Qt5 w/ OpenGL:https://blog.csdn.net/chaojiwudixiaofeixia?t=1

