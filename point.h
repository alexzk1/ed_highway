#ifndef POINT_H
#define POINT_H
#include "utils/restclient.h"
#include "utils/strfmt.h"

struct Point
{
    float x{0.f};
    float y{0.f};
    float z{0.f};

    //this assumed be start
    Point vectorTo(const Point& end) const
    {
        return {end.x - x, end.y - y, end.z - z};
    }

    float len() const //magnitude
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Point& operator*(float s)
    {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    Point& operator+(float s)
    {
        x += s;
        y += s;
        z += s;
        return *this;
    }
    Point& operator-(float s)
    {
        x -= s;
        y -= s;
        z -= s;
        return *this;
    }
    Point& operator/(float s)
    {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    Point& normalize()
    {
        const float s = 1.f / len();
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }

    Point cross(const Point& rkVector) const
    {
        return {y * rkVector.z - z * rkVector.y, z * rkVector.x - x * rkVector.z, x * rkVector.y - y * rkVector.x};
    }

    friend std::ostream & operator << (std::ostream &out, const Point &c);

    RestClient::parameters toParams() const
    {
        const static auto fmt = [](float v)
        {
            return stringfmt("%0.4f", v);
        };

        return
        {
            {"x", fmt(x)},
            {"y", fmt(y)},
            {"z", fmt(z)},
        };
    }
};

inline Point operator + (const Point& a, const Point& b)
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline Point operator - (const Point& a, const Point& b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}


//dot product (if a & b are perpendicular, dot product is zero)
inline float operator * (const Point& a, const Point& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline std::ostream & operator << (std::ostream &out, const Point &c)
{
    out << "[" << c.x << "; " << c.y << "; " << c.z << "]";
    return out;
}
#endif // POINT_H
