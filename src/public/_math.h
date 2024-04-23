#pragma once
#include "_common.h"


struct Vec2i {
    union {
        i32 elements[2]{};

        struct {
            i32 x;
            i32 y;
        };

        struct {
            i32 width;
            i32 height;
        };
    };

    consteval static Vec2i zeroed() { return Vec2i{.x = 0, .y = 0}; }
    constexpr static Vec2i splat(const i32 scalar) { return Vec2i{scalar, scalar}; }

    i32 operator[](usize index) const;
    i32& operator[](usize index);

    Vec2i operator-() const;

    // vector addition
    friend void operator+=(Vec2i& vec, Vec2i other);
    friend Vec2i operator+(Vec2i vec, Vec2i other);

    // vector subtraction
    friend void operator-=(Vec2i& vec, Vec2i other);
    friend Vec2i operator-(Vec2i vec, Vec2i other);

    // scalar multiplication
    friend void operator*=(Vec2i& vec, i32 scalar);
    friend Vec2i operator*(Vec2i vec, i32 scalar);

    // scalar division
    friend void operator/=(Vec2i& vec, i32 scalar);
    friend Vec2i operator/(Vec2i vec, i32 scalar);

    // conversions
    //
    explicit operator struct Vec2() const;
};


struct Vec2 {
    union {
        f32 elements[2]{};

        struct {
            f32 x;
            f32 y;
        };

        struct {
            f32 width;
            f32 height;
        };
    };

    consteval static Vec2 zeroed() { return Vec2{.x = 0, .y = 0}; }
    constexpr static Vec2 splat(const f32 scalar) { return Vec2{scalar, scalar}; }

    f32 operator[](usize index) const;
    f32& operator[](usize index);

    Vec2 operator-() const;

    // vector addition
    friend void operator+=(Vec2& vec, Vec2 other);
    friend Vec2 operator+(Vec2 vec, Vec2 other);

    // vector subtraction
    friend void operator-=(Vec2& vec, Vec2 other);
    friend Vec2 operator-(Vec2 vec, Vec2 other);

    // scalar multiplication
    friend void operator*=(Vec2& vec, f32 scalar);
    friend Vec2 operator*(Vec2 vec, f32 scalar);

    // scalar division
    friend void operator/=(Vec2& vec, f32 scalar);
    friend Vec2 operator/(Vec2 vec, f32 scalar);

    // conversions
    //
    explicit operator Vec2i() const;
};


struct Vec4; 


struct Vec3 {
    union {
        f32 elements[3]{};

        struct {
            f32 x;
            f32 y;
            f32 z;
        };
    };

    static consteval Vec3 zeroed() { return Vec3{0, 0, 0}; }
    static constexpr Vec3 splat(const f32 scalar) { return Vec3{scalar, scalar, scalar}; }

    static Vec3 from_vec4(const Vec4& vec);

    f32 operator[](usize index) const;
    f32& operator[](usize index);

    Vec3 operator-() const;

    // vector addition
    friend void operator+=(Vec3& vec, Vec3 other);
    friend Vec3 operator+(Vec3 vec, Vec3 other);

    // vector subtraction
    friend void operator-=(Vec3& vec, Vec3 other);
    friend Vec3 operator-(Vec3 vec, Vec3 other);

    // scalar multiplication
    friend void operator*=(Vec3& vec, f32 scalar);
    friend Vec3 operator*(Vec3 vec, f32 scalar);

    // scalar division
    friend void operator/=(Vec3& vec, f32 scalar);
    friend Vec3 operator/(Vec3 vec, f32 scalar);
};


struct Vec4 {
    union {
        f32 elements[4]{};

        struct {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
    };

    static consteval Vec4 zeroed() { return Vec4{0, 0, 0, 0}; }
    static constexpr Vec4 splat(const f32 scalar) { return Vec4{scalar, scalar, scalar, scalar}; }

    static Vec4 from_vec3(const Vec3& vec, f32 w);

    f32 operator[](usize index) const;
    f32& operator[](usize index);

    Vec4 operator-() const;

    // vector addition
    friend void operator+=(Vec4& vec, Vec4 other);
    friend Vec4 operator+(Vec4 vec, Vec4 other);

    // vector subtraction
    friend void operator-=(Vec4& vec, Vec4 other);
    friend Vec4 operator-(Vec4 vec, Vec4 other);

    // scalar multiplication
    friend void operator*=(Vec4& vec, f32 scalar);
    friend Vec4 operator*(Vec4 vec, f32 scalar);

    // scalar division
    friend void operator/=(Vec4& vec, f32 scalar);
    friend Vec4 operator/(Vec4 vec, f32 scalar);
};


struct Mat4 {
    Vec4 rows[4]{};

    static consteval Mat4 identity() {
        Mat4 mat;
        mat.rows[0] = {1, 0, 0, 0};
        mat.rows[1] = {0, 1, 0, 0};
        mat.rows[2] = {0, 0, 1, 0};
        mat.rows[3] = {0, 0, 0, 1};
        return mat;
    }

    // Assumes that the frustum is centered on the z-axis.
    static Mat4 perspective_projection(f32 fov_y, f32 aspect_ratio, f32 z_near, f32 z_far);

    static Mat4 scale(Vec3 s);
    static Mat4 translation(Vec3 t);

    static Mat4 rot_x(float angle);
    static Mat4 rot_y(float angle);
    static Mat4 rot_z(float angle);

    const Vec4& operator[](usize row_index) const;
    Vec4& operator[](usize row_index);

    friend Mat4 operator*(const Mat4& mat_a, const Mat4& mat_b);
    friend Vec4 operator*(const Mat4& mat, Vec4 vec);
};


namespace math {
    constexpr f64 pi = 3.14159265358979323846;
    constexpr f64 tau = pi * 2.0;

    f32 sq_magnitude(Vec2 vec);
    f32 magnitude(Vec2 vec);

    f32 sq_magnitude(Vec3 vec);
    f32 magnitude(Vec3 vec);

    Vec2 normalized(Vec2 vec);
    Vec3 normalized(Vec3 vec);

    f32 dot(Vec2 a, Vec2 b);
    f32 dot(Vec3 a, Vec3 b);
    f32 dot(Vec4 a, Vec4 b);
    Vec3 cross(Vec3 a, Vec3 b);

    Vec3 rot_x(Vec3 vec, f32 angle);
    Vec3 rot_y(Vec3 vec, f32 angle);
    Vec3 rot_z(Vec3 vec, f32 angle);

    f32 to_radians(f32 degrees);
    f32 to_degrees(f32 radians);

    Vec4 perspective_divide(Vec4 vec);
}
