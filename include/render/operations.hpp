#pragma once

#include "structures.hpp"
#include <b-rep.hpp>
#include <vector>

namespace render
{
    /// 提取Solid中的点
    /// @param solid 要处理的solid
    /// @return 点的集合
    /// @note sizeof(vec)=面的个数
    /// @note sizeof(vec[])=环的个数（默认vec[][0]为外环，其余为内环）
    /// @note sizeof(vec[][])=环上的点的个数
    std::vector<std::vector<std::vector<point>>> to_points(brep::solid* solid);
}
