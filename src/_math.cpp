#include "_math.h"

#include <cmath>

// =====================================================================================================================
// == i32_2 ============================================================================================================
// =====================================================================================================================

const i32& i32_2::operator[](const usize index) const {
    return elements[index];
}


i32& i32_2::operator[](const usize index) {
    return elements[index];
}


void operator+=(i32_2& vec, const i32_2& other) {
    vec.x += other.x;
    vec.y += other.y;
}


i32_2 operator+(const i32_2& vec, const i32_2& other) {
    i32_2 vec1(vec);
    vec1 += other;
    return vec1;
}


void operator-=(i32_2& vec, const i32_2& other) {
    vec.x -= other.x;
    vec.y -= other.y;
}


i32_2 operator-(const i32_2& vec, const i32_2& other) {
    i32_2 vec1(vec);
    vec1 -= other;
    return vec1;
}


void operator*=(i32_2& vec, const i32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
}


i32_2 operator*(const i32_2& vec, const i32 scalar) {
    i32_2 vec1(vec);
    vec1 *= scalar;
    return vec1;
}


void operator/=(i32_2& vec, const i32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
}


i32_2 operator/(const i32_2& vec, const i32 scalar) {
    i32_2 vec1(vec);
    vec1 /= scalar;
    return vec1;
}


i32_2::operator f32_2() const {
    return f32_2{static_cast<f32>(x), static_cast<f32>(y)};
}


// =====================================================================================================================
// == f32_2 ============================================================================================================
// =====================================================================================================================

const f32& f32_2::operator[](const size_t index) const {
    return elements[index];
}


f32& f32_2::operator[](const size_t index) {
    return elements[index];
}


void operator+=(f32_2& vec, const f32_2& other) {
    vec.x += other.x;
    vec.y += other.y;
}


f32_2 operator+(const f32_2& vec, const f32_2& other) {
    f32_2 vec1(vec);
    vec1 += other;
    return vec1;
}


void operator-=(f32_2& vec, const f32_2& other) {
    vec.x -= other.x;
    vec.y -= other.y;
}


f32_2 operator-(const f32_2& vec, const f32_2& other) {
    f32_2 vec1(vec);
    vec1 -= other;
    return vec1;
}


void operator*=(f32_2 &vec, const f32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
}


f32_2 operator*(const f32_2 &vec, const f32 scalar) {
    f32_2 vec1(vec);
    vec1 *= scalar;
    return vec1;
}


void operator/=(f32_2 &vec, const f32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
}


f32_2 operator/(const f32_2 &vec, const f32 scalar) {
    f32_2 vec1(vec);
    vec1 /= scalar;
    return vec1;
}


f32_2::operator i32_2() const {
    return i32_2{static_cast<i32>(x), static_cast<i32>(y)};
}


// =====================================================================================================================
// == f32_3 ============================================================================================================
// =====================================================================================================================

const f32 & f32_3::operator[](const size_t index) const {
    return elements[index];
}


f32& f32_3::operator[](const size_t index) {
    return elements[index];
}


void operator+=(f32_3& vec, const f32_3& other) {
    vec.x += other.x;
    vec.y += other.y;
    vec.z += other.z;
}


f32_3 operator+(const f32_3& vec, const f32_3& other) {
    f32_3 vec1(vec);
    vec1 += other;
    return vec1;
}


void operator-=(f32_3& vec, const f32_3& other) {
    vec.x -= other.x;
    vec.y -= other.y;
    vec.z -= other.z;
}


f32_3 operator-(const f32_3& vec, const f32_3& other) {
    f32_3 vec1(vec);
    vec1 -= other;
    return vec1;
}


void operator*=(f32_3 &vec, const f32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
    vec.z *= scalar;
}


f32_3 operator*(const f32_3 &vec, const f32 scalar) {
    f32_3 vec1{vec};
    vec1 *= scalar;
    return vec1;
}


void operator/=(f32_3 &vec, const f32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
    vec.z /= scalar;
}


f32_3 operator/(const f32_3 &vec, const f32 scalar) {
    f32_3 vec1{vec};
    vec1 /= scalar;
    return vec1;
}


// =====================================================================================================================
// == namespace math ===================================================================================================
// =====================================================================================================================

f32 math::sq_magnitude(const Vector2& vec) {
    return (vec.x * vec.x) + (vec.y * vec.y);
}


f32 math::magnitude(const Vector2& vec) {
    return std::sqrt(sq_magnitude(vec));
}


f32 math::sq_magnitude(const Vector3& vec) {
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}


f32 math::magnitude(const Vector3& vec) {
    return std::sqrt(sq_magnitude(vec));
}


Vector2 math::normalized(const Vector2& vec) {
    const f32 sq_length = sq_magnitude(vec);
    if (sq_length == 0.f) {
        return Vector2::zeroed();
    }
    return vec / std::sqrt(sq_length);
}


Vector3 math::normalized(const Vector3& vec) {
    const f32 sq_length = sq_magnitude(vec);
    if (sq_length == 0.f) {
        return Vector3::zeroed();
    }
    return vec / std::sqrt(sq_length);
}


f32 math::dot(const Vector2& a, const Vector2& b) {
    return a.x * b.x + a.y * b.y;
}


f32 math::dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


Vector3 math::cross(const Vector3& a, const Vector3& b) {
    return Vector3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}


Vector3 math::rot_x(const Vector3& vec, const f32 angle) {
    return Vector3{
        vec.x,
        vec.y * std::cos(angle) + vec.z * std::sin(angle),
        vec.z * std::cos(angle) - vec.y * std::sin(angle),
    };
}


Vector3 math::rot_y(const Vector3& vec, const f32 angle) {
    return Vector3{
        vec.x * std::cos(angle) - vec.z * std::sin(angle),
        vec.y,
        vec.z * std::cos(angle) + vec.x * std::sin(angle),
    };
}


Vector3 math::rot_z(const Vector3& vec, const f32 angle) {
    return Vector3{
        vec.x * std::cos(angle) - vec.y * std::sin(angle),
        vec.y * std::cos(angle) + vec.x * std::sin(angle),
        vec.z
    };
}
