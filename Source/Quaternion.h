#pragma once

struct Quaternion {
    double w;
    double x;
    double y;
    double z;

    // Basic operations
    double Length() const;
    Quaternion Normalize();

    // Complex operations
    Quaternion Mul(const Quaternion& other) const;
    Quaternion Rotate90();
};
