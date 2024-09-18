// sphere.h
#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"

class Sphere : public Hittable {
public:
    Sphere() {}
    Sphere(Point3 cen, double r, std::shared_ptr<Material> m)
        : center(cen), radius(r), material_ptr(m) {}

    virtual bool hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const override;

public:
    Point3 center;
    double radius;
    std::shared_ptr<Material> material_ptr;
};

inline bool Sphere::hit(const Ray& r, double t_min, double t_max, HitRecord& rec) const {
    Vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant > 0) {
        auto root = sqrt(discriminant);

        auto temp = (-half_b - root) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            Vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.material_ptr = material_ptr;
            return true;
        }

        temp = (-half_b + root) / a;
        if (temp < t_max && temp > t_min) {
            rec.t = temp;
            rec.p = r.at(rec.t);
            Vec3 outward_normal = (rec.p - center) / radius;
            rec.set_face_normal(r, outward_normal);
            rec.material_ptr = material_ptr;
            return true;
        }
    }
    return false;
}

#endif // SPHERE_H
