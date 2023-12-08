#ifndef __VEC2D__
#define __VEC2D__

#include <iostream>

class Vec2D
{
public:
    Vec2D() : m_x(0.0f), m_y(0.0f) {}
    Vec2D(float x, float y) : m_x(x), m_y(y) {}

    float getX() const { return m_x; }
    float getY() const { return m_y; }

    Vec2D operator+(const Vec2D &other) const
    {
        return Vec2D(m_x + other.getX(), m_y + other.getY());
    }

    friend Vec2D operator*(const float &scalar, const Vec2D &vec)
    {
        return Vec2D(scalar * vec.m_x, scalar * vec.m_y);
    }

    Vec2D operator-(const Vec2D &other) const
    {
        return Vec2D(m_x - other.getX(), m_y - other.getY());
    }

    friend Vec2D operator/(const Vec2D &vec, const float &scalar)
    {
        if (scalar != 0)
        {
            return (1 / scalar) * vec;
        }
        std::cout << "Impossible to divide by zero !" << '\n';
        return vec;
    }

    void operator+=(const Vec2D &other)
    {
        m_x += other.getX();
        m_y += other.getY();
    }

    void operator-=(const Vec2D &other)
    {
        m_x -= other.getX();
        m_y -= other.getY();
    }

    void operator*=(const float &scalar)
    {
        m_x *= scalar;
        m_y *= scalar;
    }

    void operator/=(const float &scalar)
    {
        m_x /= scalar;
        m_y /= scalar;
    }

    float length() const
    {
        return sqrtf(m_x * m_x + m_y * m_y);
    }

    float length_squared() const
    {
        return m_x * m_x + m_y * m_y;
    }

    void normalize_emplace()
    {
        *(this) /= this->length();
    }

    Vec2D normalize() const
    {
        return Vec2D(m_x / this->length(), m_y / this->length());
    }

private:
    float m_x;
    float m_y;
};

#endif