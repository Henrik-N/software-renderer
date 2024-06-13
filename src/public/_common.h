#pragma once

#include <array>
#include <cmath>
#include <string>
#include <vector>
#include <ostream>
#include <iostream>


using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float_t;
using f64 = double_t;
using usize = size_t;


#if PLATFORM_LINUX && COLORED_LOG

#define INFO( msg ) std::cout << "\e[32m[INFO]\e[0m \e[36m" << __FUNCTION__ << "\e[0m --> " << msg << std::endl;
#define WARN( msg ) std::cerr << "\e[31m[WARNING]\e[0m \e[36m" << __FUNCTION__ << "\e[0m --> " << msg << std::endl;
#define ERR( msg ) std::cerr << "\e[31m[ERROR]\e[0m \e[36m" << __FUNCTION__ << "\e[0m --> " << msg << std::endl;

#else

#define INFO( msg ) std::cout << "[INFO] " <<  __FUNCTION__ << " --> " << msg << std::endl;
#define WARN( msg ) std::cerr << "[WARNING]" << __FUNCTION__ << " --> " << msg << std::endl;
#define ERR( msg ) std::cerr << "[ERROR] " << __FUNCTION__ << " --> " << msg << std::endl;

#endif


#define ENUM_CASE_RETURN_AS_STRING(x) case x: return #x;
