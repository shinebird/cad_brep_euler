#pragma once

#include <iostream>
#include <vectorclass.h>

namespace render
{
    class point
    {
        public:
            double coord[3];
            point() : coord() {}
            // point(int x, int y, int z) { set_coord(x, y, z); }
            point(double x, double y, double z) : coord{x, y, z} {}
            point(double* coords) : coord{coords[0], coords[1], coords[2]} {}
            double* GetCoord() { return coord; }
            void set_coord(double x, double y, double z)
            {
                vcl::Vec4d vec = {x, y, z, 0};
                vec.store_partial(3, this->coord);
            }
            void set_coord(double input_coord[3])
            {
                vcl::Vec4d vec;
                vec.load_partial(3, input_coord);
                vec.store_partial(3, this->coord);
            }
            void set_coord(const point& point)
            {
                vcl::Vec4d vec;
                vec.load_partial(3, point.coord);
                vec.store_partial(3, this->coord);
            }
            friend inline std::istream& operator>>(std::istream& is,
                                                   point& point)
            {
                is >> point.coord[0] >> point.coord[1] >> point.coord[2];
                return is;
            }
            friend inline std::ostream& operator<<(std::ostream& os,
                                                   point& point)
            {
                os << "( " << point.coord[0] << ", " << point.coord[1] << ", "
                   << point.coord[2] << " )";
                return os;
            }
            bool operator==(const point& another_point) const
            {
                return std::abs(this->coord[0] - another_point.coord[0]) <
                           0.00001 &&
                       std::abs(this->coord[1] - another_point.coord[1]) <
                           0.00001 &&
                       std::abs(this->coord[2] - another_point.coord[2]) <
                           0.00001;
            }
    };
} // namespace render
