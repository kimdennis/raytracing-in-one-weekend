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

    if (discriminant >= 0) {
        auto sqrt_discriminant = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrt_discriminant) / a;
        if (root < t_min || root > t_max) {
            root = (-half_b + sqrt_discriminant) / a;
            if (root < t_min || root > t_max)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);

        // Adjusted normal calculation
        Vec3 outward_normal = (rec.p - center) / fabs(radius);

        rec.set_face_normal(r, outward_normal);
        rec.material_ptr = material_ptr;
        return true;
    }
    return false;
}

#endif // SPHERE_H
