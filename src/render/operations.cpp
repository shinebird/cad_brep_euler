#include <render.hpp>

#include <algorithm>
#include <unordered_set>

namespace render
{
    std::vector<std::vector<std::vector<point>>> to_points(brep::solid* solid)
    {
        std::vector<std::vector<std::vector<point>>> result;
        for (auto face = solid->sface; face != nullptr; face = face->nextf)
        {
            std::vector<std::vector<point>> face_vector;
            for (auto loop = face->floops; loop != nullptr; loop = loop->nextl)
            {
                std::vector<point> loop_vector;
                auto start_half_edge = loop->ledge;
                auto half_edge = loop->ledge;
                do
                {
                    loop_vector.emplace_back(*(half_edge->startv->point));
                    half_edge = half_edge->next;
                }
                while (half_edge != nullptr && half_edge != start_half_edge);

                // 点的去重
                std::unordered_set<point, decltype([](const point& p) {return std::hash<double>()(p.coord[0]) ^ std::hash<double>()(p.coord[1]) ^ std::hash<double>()(p.coord[2]);})> point_set(loop_vector.begin(), loop_vector.end());
                std::vector<point> loop_vector1;
                for (std::size_t i = 0; i < loop_vector.size() &&
                                        loop_vector1.size() <= point_set.size();
                     i++)
                {
                    if (point_set.contains(loop_vector[i]))
                    {
                        loop_vector1.emplace_back(loop_vector[i]);
                    }
                }

                // 排除邻接半边的影响
                for (std::size_t i = 0; i + 2 < loop_vector1.size(); i++)
                {
                    if (loop_vector1[i] == loop_vector1[i + 2])
                    {
                        loop_vector1.erase(loop_vector1.begin(),
                                           loop_vector1.begin() + i + 1);
                    }
                }
                face_vector.emplace_back(loop_vector1);
            }
            result.emplace_back(face_vector);
        }
        return result;
    }

} // namespace render
