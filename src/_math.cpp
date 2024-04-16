#include "_math.h"

#include <cmath>


// =====================================================================================================================
// == Vec2i ============================================================================================================
// =====================================================================================================================

i32 Vec2i::operator[](const usize index) const {
    return elements[index];
}


i32& Vec2i::operator[](const usize index) {
    return elements[index];
}


void operator+=(Vec2i& vec, const Vec2i other) {
    vec.x += other.x;
    vec.y += other.y;
}


Vec2i operator+(Vec2i vec, const Vec2i other) {
    vec += other;
    return vec;
}


void operator-=(Vec2i& vec, const Vec2i other) {
    vec.x -= other.x;
    vec.y -= other.y;
}


Vec2i operator-(Vec2i vec, const Vec2i other) {
    vec -= other;
    return vec;
}


void operator*=(Vec2i& vec, const i32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
}


Vec2i operator*(Vec2i vec, const i32 scalar) {
    vec *= scalar;
    return vec;
}


void operator/=(Vec2i& vec, const i32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
}


Vec2i operator/(Vec2i vec, const i32 scalar) {
    vec /= scalar;
    return vec;
}


Vec2i::operator Vec2() const {
    return Vec2{static_cast<f32>(x), static_cast<f32>(y)};
}


// =====================================================================================================================
// == Vec2 ============================================================================================================
// =====================================================================================================================

f32 Vec2::operator[](const usize index) const {
    return elements[index];
}


f32& Vec2::operator[](const usize index) {
    return elements[index];
}


void operator+=(Vec2& vec, const Vec2 other) {
    vec.x += other.x;
    vec.y += other.y;
}


Vec2 operator+(Vec2 vec, const Vec2 other) {
    vec += other;
    return vec;
}


void operator-=(Vec2& vec, const Vec2 other) {
    vec.x -= other.x;
    vec.y -= other.y;
}


Vec2 operator-(Vec2 vec, const Vec2 other) {
    vec -= other;
    return vec;
}


void operator*=(Vec2 &vec, const f32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
}


Vec2 operator*(Vec2 vec, const f32 scalar) {
    vec *= scalar;
    return vec;
}


void operator/=(Vec2 &vec, const f32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
}


Vec2 operator/(Vec2 vec, const f32 scalar) {
    vec /= scalar;
    return vec;
}


Vec2::operator Vec2i() const {
    return Vec2i{static_cast<i32>(x), static_cast<i32>(y)};
}


// =====================================================================================================================
// == Vec3 ============================================================================================================
// =====================================================================================================================

Vec3 Vec3::from_vec4(const Vec4& vec) {
    return Vec3{vec.x, vec.y, vec.z};
}


f32 Vec3::operator[](const usize index) const {
    return elements[index];
}


f32& Vec3::operator[](const usize index) {
    return elements[index];
}


void operator+=(Vec3& vec, const Vec3 other) {
    vec.x += other.x;
    vec.y += other.y;
    vec.z += other.z;
}


Vec3 operator+(Vec3 vec, const Vec3 other) {
    vec += other;
    return vec;
}


void operator-=(Vec3& vec, const Vec3 other) {
    vec.x -= other.x;
    vec.y -= other.y;
    vec.z -= other.z;
}


Vec3 operator-(Vec3 vec, const Vec3 other) {
    vec -= other;
    return vec;
}


void operator*=(Vec3 &vec, const f32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
    vec.z *= scalar;
}


Vec3 operator*(Vec3 vec, const f32 scalar) {
    vec *= scalar;
    return vec;
}


void operator/=(Vec3 &vec, const f32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
    vec.z /= scalar;
}


Vec3 operator/(Vec3 vec, const f32 scalar) {
    vec /= scalar;
    return vec;
}


// =====================================================================================================================
// == Vec4 ============================================================================================================
// =====================================================================================================================

Vec4 Vec4::from_vec3(const Vec3& vec, f32 w) {
    return Vec4{vec.x, vec.y, vec.z, w};
}


f32 Vec4::operator[](const usize index) const {
    return elements[index];
}


f32& Vec4::operator[](const usize index) {
    return elements[index];
}


void operator+=(Vec4& vec, const Vec4 other) {
    vec.x += other.x;
    vec.y += other.y;
    vec.z += other.z;
    vec.w += other.w;
}


Vec4 operator+(Vec4 vec, const Vec4 other) {
    vec += other;
    return vec;
}


void operator-=(Vec4& vec, const Vec4 other) {
    vec.x -= other.x;
    vec.y -= other.y;
    vec.z -= other.z;
    vec.w -= other.w;
}


Vec4 operator-(Vec4 vec, const Vec4 other) {
    vec -= other;
    return vec;
}


void operator*=(Vec4& vec, const f32 scalar) {
    vec.x *= scalar;
    vec.y *= scalar;
    vec.z *= scalar;
    vec.w *= scalar;
}


Vec4 operator*(Vec4 vec, const f32 scalar) {
    vec *= scalar;
    return vec;
}


void operator/=(Vec4& vec, const f32 scalar) {
    vec.x /= scalar;
    vec.y /= scalar;
    vec.z /= scalar;
    vec.w /= scalar;
}


Vec4 operator/(Vec4 vec, const f32 scalar) {
    vec /= scalar;
    return vec;
}


// =====================================================================================================================
// == Mat4 =============================================================================================================
// =====================================================================================================================

Mat4 Mat4::scale(const Vec3 s) {
    Mat4 mat; 
    mat.rows[0] = {s.x, 0,   0,   0}; 
    mat.rows[1] = {0,   s.y, 0,   0}; 
    mat.rows[2] = {0,   0,   s.z, 0}; 
    mat.rows[3] = {0,   0,   0,   1};
    return mat;
}


Mat4 Mat4::translation(const Vec3 t) {
    Mat4 mat; 
    mat.rows[0] = {1, 0, 0, t.x}; 
    mat.rows[1] = {0, 1, 0, t.y}; 
    mat.rows[2] = {0, 0, 1, t.z}; 
    mat.rows[3] = {0, 0, 0, 1  };
    return mat;
}


Mat4 Mat4::rot_x(const float angle) {
    Mat4 mat;
    mat.rows[0] = {1,  0,               0,               0};
    mat.rows[1] = {0,  std::cos(angle), std::sin(angle), 0};
    mat.rows[2] = {0, -std::sin(angle), std::cos(angle), 0};
    mat.rows[3] = {0,  0,               0,               1};
    return mat;
}


Mat4 Mat4::rot_y(const float angle) {
    Mat4 mat;
    mat.rows[0] = {std::cos(angle), 0, -std::sin(angle), 0};
    mat.rows[1] = {0,               1, 0,                0};
    mat.rows[2] = {std::sin(angle), 0, std::cos(angle),  0};
    mat.rows[3] = {0,               0, 0,                1};
    return mat;
}


Mat4 Mat4::rot_z(const float angle) {
    Mat4 mat;
    mat.rows[0] = {std::cos(angle), -std::sin(angle), 0, 0};
    mat.rows[1] = {std::sin(angle),  std::cos(angle), 0, 0};
    mat.rows[2] = {0,                0,               1, 0};
    mat.rows[3] = {0,                0,               0, 1};
    return mat;
}


const Vec4& Mat4::operator[](const usize row_index) const {
    return rows[row_index];
}


Vec4& Mat4::operator[](const usize row_index){
    return rows[row_index];
}


Mat4 operator*(const Mat4& a, const Mat4& b) {
    Mat4 mat;
    for (i32 row = 0; row < 4; ++row) {
        for (i32 col = 0; col < 4; ++col) {
            mat.rows[row][col] = a[row][0] * b[0][col] + 
                                 a[row][1] * b[1][col] +
                                 a[row][2] * b[2][col] +
                                 a[row][3] * b[3][col];
        }
    }
    return mat;
}


Vec4 operator*(const Mat4& mat, const Vec4 vec) {
    Vec4 result;
    result.x = math::dot(mat.rows[0], vec);
    result.y = math::dot(mat.rows[1], vec);
    result.z = math::dot(mat.rows[2], vec);
    result.w = math::dot(mat.rows[3], vec);
    return result;
}


// =====================================================================================================================
// == namespace math ===================================================================================================
// =====================================================================================================================

f32 math::sq_magnitude(const Vec2 vec) {
    return (vec.x * vec.x) + (vec.y * vec.y);
}


f32 math::magnitude(const Vec2 vec) {
    return std::sqrt(sq_magnitude(vec));
}


f32 math::sq_magnitude(const Vec3 vec) {
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}


f32 math::magnitude(const Vec3 vec) {
    return std::sqrt(sq_magnitude(vec));
}


Vec2 math::normalized(const Vec2 vec) {
    const f32 sq_length = sq_magnitude(vec);
    if (sq_length == 0.f) {
        return Vec2::zeroed();
    }
    return vec / std::sqrt(sq_length);
}


Vec3 math::normalized(const Vec3 vec) {
    const f32 sq_length = sq_magnitude(vec);
    if (sq_length == 0.f) {
        return Vec3::zeroed();
    }
    return vec / std::sqrt(sq_length);
}


f32 math::dot(const Vec2 a, const Vec2 b) {
    return a.x * b.x + a.y * b.y;
}


f32 math::dot(const Vec3 a, const Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}


f32 math::dot(const Vec4 a, const Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}


Vec3 math::cross(const Vec3 a, const Vec3 b) {
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}


Vec3 math::rot_x(const Vec3 vec, const f32 angle) {
    return Vec3{
        vec.x,
        vec.y * std::cos(angle) + vec.z * std::sin(angle),
        vec.z * std::cos(angle) - vec.y * std::sin(angle),
    };
}


Vec3 math::rot_y(const Vec3 vec, const f32 angle) {
    return Vec3{
        vec.x * std::cos(angle) - vec.z * std::sin(angle),
        vec.y,
        vec.z * std::cos(angle) + vec.x * std::sin(angle),
    };
}


Vec3 math::rot_z(const Vec3 vec, const f32 angle) {
    return Vec3{
        vec.x * std::cos(angle) - vec.y * std::sin(angle),
        vec.y * std::cos(angle) + vec.x * std::sin(angle),
        vec.z
    };
}
