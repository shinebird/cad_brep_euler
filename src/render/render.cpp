#include <render.hpp>
#include <vectorclass.h>

namespace render
{
    std::shared_ptr<double[]> get_normal(double* p1, double* p2, double* p3)
    {
        vcl::Vec4d A, B, C;
        A.load_partial(3, p1);
        B.load_partial(3, p2);
        C.load_partial(3, p3);
        auto delta1 = B - A;
        auto delta2 = C - A;
        double delta1_data[3], delta2_data[3];
        delta1.store_partial(3, delta1_data);
        delta2.store_partial(3, delta2_data);
        std::shared_ptr<double[]> result(new double[3]);
        result[0] =
            delta1_data[1] * delta2_data[2] - delta1_data[2] * delta2_data[1];
        result[1] =
            delta1_data[2] * delta2_data[0] - delta1_data[0] * delta2_data[2];
        result[2] =
            delta1_data[0] * delta2_data[1] - delta1_data[1] * delta1_data[0];
        return result;
    }
} // namespace render
