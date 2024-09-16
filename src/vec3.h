#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

class Vec3 {
public:
    double e[3];

    Vec3();
    Vec3(double e0, double e1, double e2);

    double x() const;
    double y() const;
    double z() const;

    Vec3 operator-() const;
    double operator[](int i) const;
    double& operator[](int i);

    Vec3& operator+=(const Vec3& v);
    Vec3& operator*=(const double t);
    Vec3& operator/=(const double t);

    double length() const;
    double length_squared() const;

    bool near_zero() const;
};

// Utility functions for Vec3
std::ostream& operator<<(std::ostream& out, const Vec3& v);
Vec3 operator+(const Vec3& u, const Vec3& v);
Vec3 operator-(const Vec3& u, const Vec3& v);
Vec3 operator*(const Vec3& u, const Vec3& v);
Vec3 operator*(double t, const Vec3& v);
Vec3 operator*(const Vec3& v, double t);
Vec3 operator/(Vec3 v, double t);
double dot(const Vec3& u, const Vec3& v);
Vec3 cross(const Vec3& u, const Vec3& v);
Vec3 unit_vector(Vec3 v);

// Type aliases for 3D points and RGB colors
using Point3 = Vec3;   // Represents a point in 3D space
using Color = Vec3;    // Represents an RGB color

#endif
