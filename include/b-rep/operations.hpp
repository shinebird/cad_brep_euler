#pragma once

#include "structures.hpp"
#include <render/structures.hpp>

namespace brep
{
    /// 构造一个体，一个面，一个外环，一个点
    /// @param point 点的坐标
    /// @param newVertex 构造的点
    /// @return 构造的体
    solid* mvfs(render::point* point, vertex** newVertex);

    /// 构造一个新点，同时构造一条连接新点与一给定点的边
    /// @param v1 要连接的点
    /// @param p2 点的坐标
    /// @param lp 要处理的环
    /// @return 生成的半边（之一）
    half_edge* mev(vertex* v1, render::point* p2, loop* lp);

    /// 构造一个边，一个面，一个环
    /// @param v1 要连接的点之一
    /// @param v2 要连接的点之一
    /// @param lp 要处理的环
    /// @return 生成的新环
    loop* mef(vertex* v1, vertex* v2, loop* lp);

    /// 删除一条边构造一个环
    /// @param v1 要删除的边的一个端点
    /// @param v2 要删除的边的另一个端点
    /// @param lp 要处理的环
    /// @return 生成的新环
    loop* kemr(vertex* v1, vertex* v2, loop* lp);

    /// 删除一个与面f1相接触的一个面f2，声称面f1上的一个面上内环，并形成体上一个通孔
    /// @param out_lp f1上的一个环
    /// @param lp f2上的一个环
    void kfmrh(loop* out_lp, loop* lp);

    /// 扫成操作
    /// @param face 底面
    /// @param vec 扫成的方向
    /// @param distance 扫成的距离
    void sweep(face* face, double vec[3], double distance);
} // namespace brep
