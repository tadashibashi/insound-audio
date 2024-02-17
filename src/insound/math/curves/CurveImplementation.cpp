#include "QuadBezierCurve.h"
#include "CubicBezierCurve.h"

#include <cmath>

namespace Insound::Math
{
    Vec2f QuadBezierCurve::value(float t) const
    {
        auto p = m_points;
        auto omt = 1-t;
        return  (omt*omt * p[0]) +
            (2 * omt * t * p[1]) +
            (t * t * p[2]);
    }

    Vec2f CubicBezierCurve::value(float t) const
    {
        auto p = m_points;
        auto omt = 1-t;
        return (omt*omt*omt * p[0]) +
            (3 * omt*omt * t * p[1]) +
            (3 * omt * t*t * p[2]) +
            (t*t*t * p[3]);
    }
}
