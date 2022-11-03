#pragma once
#include <memory>

namespace render
{
    /// 计算三角形的法向
    /// @param p1 顶点坐标(x1, y1, z1)
    /// @param p2 顶点坐标(x2, y2, z2)
    /// @param p3 顶点坐标(x3, y3, z3)
    std::shared_ptr<double[]> get_normal(double* p1, double* p2, double* p3);

} // namespace render
