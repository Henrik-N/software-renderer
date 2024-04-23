#include "_color.h"


// =====================================================================================================================
// == Color ============================================================================================================
// =====================================================================================================================

Color Color::with_intensity(const float percentage) const {
    return Color{
        .b = static_cast<u8>(static_cast<f32>(b) * percentage),
        .g = static_cast<u8>(static_cast<f32>(g) * percentage),
        .r = static_cast<u8>(static_cast<f32>(r) * percentage),
        .a = a,
    };
}
