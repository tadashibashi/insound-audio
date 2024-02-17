#pragma once
#include <cmath>
#include <stdexcept>
#include <type_traits>

template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

namespace Insound::Math
{
    template <Arithmetic T>
    class Vec2_
    {
    public:
        Vec2_(T x, T y): x{x}, y{y} {}
        Vec2_(): x{0}, y{0} { }

        T x, y;

        /**
         * Distance from point {0, 0}
         */
        double length() const
        {
            return std::sqrt(x*x + y*y);
        }

        /**
         * Distance between this and another vector
         * @param  other - the other vector to measure distance from
         * @return       distance between this and `other` vector
         */
        double distance(const Vec2_ &other) const
        {
            double a = x - other.x;
            double b = y - other.y;
            return std::sqrt(a*a + b*b);
        }

        /**
         * Return a normalized copy of this vector (length set to 1)
         */
        Vec2_<double> normalized() const
        {
            const auto length = this->length();
            return (length == 0) ? Vec2_<T>() :
                Vec2_<double>(*this) /= length;
        }

        template <Arithmetic U>
        Vec2_ &operator=(const Vec2_<U> &other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        Vec2_ &operator *=(double n)
        {
            x *= n;
            y *= n;
            return *this;
        }

        Vec2_ &operator /=(double n)
        {
            if (n == 0)
            {
                throw std::runtime_error("Division by zero");
            }

            x /= n;
            y /= n;

            return *this;
        }

        template <Arithmetic U>
        Vec2_ &operator +=(const Vec2_<U> &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        template <Arithmetic U>
        Vec2_ &operator -=(const Vec2_<U> &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        template <Arithmetic U>
        Vec2_ &operator *=(const Vec2_<U> &other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        template <Arithmetic U>
        Vec2_ &operator /=(const Vec2_<U> &other)
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        template <Arithmetic U>
        operator Vec2_<U>()
        {
            return Vec2_<U>(x, y);
        }
    };

    using Vec2 = Vec2_<double>;
    using Vec2f = Vec2_<float>;
    using Vec2i = Vec2_<int>;

    template <Arithmetic T, Arithmetic U>
    inline Vec2_<T> operator+(const Vec2_<T> &a, const Vec2_<U> &b)
    {
        return Vec2_<T>(a) += b;
    }

    template <Arithmetic T, Arithmetic U>
    inline Vec2_<T> operator-(const Vec2_<T> &a, const Vec2_<U> &b)
    {
        return Vec2_<T>(a) -= b;
    }

    template <Arithmetic T, Arithmetic U>
    inline Vec2_<T> operator*(const Vec2_<T> &a, const Vec2_<U> &b)
    {
        return Vec2_<T>(a) *= b;
    }

    template <Arithmetic T, Arithmetic U>
    inline Vec2_<T> operator/(const Vec2_<T> &a, const Vec2_<U> &b)
    {
        return Vec2_<T>(a) /= b;
    }

    template <Arithmetic T>
    inline Vec2_<T> operator *(const Vec2_<T> &v, double n)
    {
        return Vec2_<T>(v) *= n;
    }

    template <Arithmetic T>
    inline Vec2_<T> operator *(double n, const Vec2_<T> &v)
    {
        return v * n;
    }

    template <Arithmetic T>
    inline Vec2_<T> operator /(const Vec2_<T> &v, double n)
    {
        return Vec2_<T>(v) /= n;
    }

    template <Arithmetic T>
    inline Vec2_<T> operator /(double n, const Vec2_<T> &v)
    {
        return v / n;
    }
}
