#pragma once

#include "_common.h"

struct Color {
    union {
        u32 hex{};

        struct {
            u8 a;
            u8 r;
            u8 g;
            u8 b;
        };
    };

    constexpr static Color rgba(const u8 r, const u8 g, const u8 b, const u8 a) {
        return Color{.a = a, .r = r, .g = g, .b = b};
    }

    consteval static Color black() { return Color{.hex = 0xFF000000}; }
    consteval static Color white() { return Color{.hex = 0xFFFFFFFF}; }
    consteval static Color grey() { return Color{.hex = 0xFF333333}; }
    consteval static Color red() { return Color{.hex = 0xFFFF0000}; }
    consteval static Color green() { return Color{.hex = 0xFF00FF00}; }
    consteval static Color blue() { return Color{.hex = 0xFF0000FF}; }
    consteval static Color yellow() { return Color{.hex = 0xFFFFFF00}; }
};
