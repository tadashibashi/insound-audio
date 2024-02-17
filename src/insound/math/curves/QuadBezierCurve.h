#pragma once

#include "Curve.h"

namespace Insound::Math
{
    /**
     * Three-point bezier curve
     */
    class QuadBezierCurve : public Curve
    {
    public:
        QuadBezierCurve() : m_points{} { }

        QuadBezierCurve(Vec2f start, Vec2f end, Vec2f control)
            : m_points{start, control, end}
        {}

        QuadBezierCurve(Vec2f start, Vec2f end)
        {}

        void setPoint(int index, Vec2f p) override
        {
            m_points[index] = p;
        }

        [[nodiscard]]
        Vec2f getPoint(int index) const override
        {
            return m_points[index];
        }

        [[nodiscard]]
        int pointCount() const override { return 3; }

        /**
         * Get the value of a specific point on the curve
         * @param  t - must be 0 <= t <= 1
         */
        [[nodiscard]]
        Vec2f value(float t) const override;

    private:
        Vec2f m_points[3];
    };
}
