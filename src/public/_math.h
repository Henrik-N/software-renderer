#pragma once
#include "_common.h"

struct i32_2 {
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

    consteval static i32_2 zeroed() { return i32_2{.x = 0, .y = 0}; }
    constexpr static i32_2 splat(const i32 scalar) { return i32_2{scalar, scalar}; }

    const i32& operator[](usize index) const;
    i32& operator[](usize index);

    // vector addition
    friend void operator+=(i32_2& vec, const i32_2& other);
    friend i32_2 operator+(const i32_2& vec, const i32_2& other);

    // vector subtraction
    friend void operator-=(i32_2& vec, const i32_2& other);
    friend i32_2 operator-(const i32_2& vec, const i32_2& other);

    // scalar multiplication
    friend void operator*=(i32_2& vec, i32 scalar);
    friend i32_2 operator*(const i32_2& vec, i32 scalar);

    // scalar division
    friend void operator/=(i32_2& vec, i32 scalar);
    friend i32_2 operator/(const i32_2& vec, i32 scalar);

    // conversions
    //
    explicit operator struct f32_2() const;
};


struct f32_2 {
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

    consteval static f32_2 zeroed() { return f32_2{.x = 0, .y = 0}; }
    constexpr static f32_2 splat(const f32 scalar) { return f32_2{scalar, scalar}; }

    const f32& operator[](size_t index) const;
    f32& operator[](size_t index);

    // vector addition
    friend void operator+=(f32_2& vec, const f32_2& other);
    friend f32_2 operator+(const f32_2& vec, const f32_2& other);

    // vector subtraction
    friend void operator-=(f32_2& vec, const f32_2& other);
    friend f32_2 operator-(const f32_2& vec, const f32_2& other);

    // scalar multiplication
    friend void operator*=(f32_2& vec, f32 scalar);
    friend f32_2 operator*(const f32_2& vec, f32 scalar);

    // scalar division
    friend void operator/=(f32_2& vec, f32 scalar);
    friend f32_2 operator/(const f32_2& vec, f32 scalar);

    // conversions
    //
    explicit operator i32_2() const;
};


using Vector2 = f32_2;


struct f32_3 {
    union {
        f32 elements[3]{};

        struct {
            f32 x;
            f32 y;
            f32 z;
        };
    };

    static consteval f32_3 zeroed() { return f32_3{0, 0, 0}; }
    static constexpr f32_3 splat(const f32 scalar) { return f32_3{scalar, scalar, scalar}; }

    const f32& operator[](size_t index) const;
    f32& operator[](size_t index);

    // vector addition
    friend void operator+=(f32_3& vec, const f32_3& other);
    friend f32_3 operator+(const f32_3& vec, const f32_3& other);

    // vector subtraction
    friend void operator-=(f32_3& vec, const f32_3& other);
    friend f32_3 operator-(const f32_3& vec, const f32_3& other);

    // scalar multiplication
    friend void operator*=(f32_3& vec, f32 scalar);
    friend f32_3 operator*(const f32_3& vec, f32 scalar);

    // scalar division
    friend void operator/=(f32_3& vec, f32 scalar);
    friend f32_3 operator/(const f32_3& vec, f32 scalar);
};


using Vector3 = f32_3;


namespace math {
    constexpr f64 pi = 3.14159265358979323846;
    constexpr f64 tau = pi * 2.0;

    f32 sq_magnitude(const Vector2& vec);
    f32 magnitude(const Vector2& vec);

    f32 sq_magnitude(const Vector3& vec);
    f32 magnitude(const Vector3& vec);

    Vector2 normalized(const Vector2& vec);
    Vector3 normalized(const Vector3& vec);

    f32 dot(const Vector2& a, const Vector2& b);
    f32 dot(const Vector3& a, const Vector3& b);
    Vector3 cross(const Vector3& a, const Vector3& b);

    Vector3 rot_x(const Vector3& vec, f32 angle);
    Vector3 rot_y(const Vector3& vec, f32 angle);
    Vector3 rot_z(const Vector3& vec, f32 angle);
}
