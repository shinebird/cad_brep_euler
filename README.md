# cad_brep_euler

## brep and euler operation

### 三维CAD建模大程Project

**林瑞健 22251135**

使用cmake进行环境配置

效果图与程序见results文件夹

该工程在Windows上使用Clang-Cl 16编译，其余平台与编译器未测试。

**程序结构：**

* include文件夹中包含头文件
* src文件夹中包含cpp文件
* b-rep文件夹为欧拉操作相关的文件
* render文件夹为绘制相关的文件

**程序控制：**

* D键用于切换线框模式、填充模式、顶点模式（顶点模式可能看不清）
* 鼠标控制相机朝向
* 按Esc退出程序

程序使用OpenGL自带的GLUtesselator对多边形进行三角化。在填充模式中插入法向时出现了未知错误（可能有部分三角片法向错误，绘制模式为GL_TRIANGLE_STRIP）。该错误并非欧拉操作出错。欧拉操作的正确性可由线框模式确认。

程序展示了两个由欧拉操作生成的几何体。左边为含通孔的长方形，右边为含通孔的长方形进行扫成操作生成的含通孔的长方体。
