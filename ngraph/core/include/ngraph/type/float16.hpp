// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "ngraph/ngraph_visibility.hpp"

#define ROUND_MODE_TO_NEAREST_EVEN

namespace ngraph {
class NGRAPH_API float16 {
public:
    constexpr float16() : m_value{0} {}

    static uint32_t constexpr frac_size = 10;
    static uint32_t constexpr exp_size = 5;
    static uint32_t constexpr exp_bias = 15;

    float16(uint32_t sign, uint32_t biased_exponent, uint32_t fraction)
        : m_value((sign & 0x01) << 15 | (biased_exponent & 0x1F) << 10 | (fraction & 0x03FF)) {}

    float16(float value);

    template <typename I>
    explicit float16(I value) : m_value{float16{static_cast<float>(value)}.m_value} {}

    std::string to_string() const;
    size_t size() const;
    template <typename T>
    bool operator==(const T& other) const;
    template <typename T>
    bool operator!=(const T& other) const {
        return !(*this == other);
    }
    template <typename T>
    bool operator<(const T& other) const;
    template <typename T>
    bool operator<=(const T& other) const;
    template <typename T>
    bool operator>(const T& other) const;
    template <typename T>
    bool operator>=(const T& other) const;
    template <typename T>
    float16 operator+(const T& other) const;
    template <typename T>
    float16 operator+=(const T& other);
    template <typename T>
    float16 operator-(const T& other) const;
    template <typename T>
    float16 operator-=(const T& other);
    template <typename T>
    float16 operator*(const T& other) const;
    template <typename T>
    float16 operator*=(const T& other);
    template <typename T>
    float16 operator/(const T& other) const;
    template <typename T>
    float16 operator/=(const T& other);
    operator float() const;

    static constexpr float16 from_bits(uint16_t bits) {
        return float16(bits, true);
    }
    uint16_t to_bits() const;
    friend std::ostream& operator<<(std::ostream& out, const float16& obj) {
        out << static_cast<float>(obj);
        return out;
    }

private:
    constexpr float16(uint16_t x, bool) : m_value{x} {}
    union F32 {
        F32(float val) : f{val} {}
        F32(uint32_t val) : i{val} {}
        float f;
        uint32_t i;
    };

    uint16_t m_value;
};

template <typename T>
bool float16::operator==(const T& other) const {
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    return (static_cast<float>(*this) == static_cast<float>(other));
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
}

template <typename T>
bool float16::operator<(const T& other) const {
    return (static_cast<float>(*this) < static_cast<float>(other));
}

template <typename T>
bool float16::operator<=(const T& other) const {
    return (static_cast<float>(*this) <= static_cast<float>(other));
}

template <typename T>
bool float16::operator>(const T& other) const {
    return (static_cast<float>(*this) > static_cast<float>(other));
}

template <typename T>
bool float16::operator>=(const T& other) const {
    return (static_cast<float>(*this) >= static_cast<float>(other));
}

template <typename T>
float16 float16::operator+(const T& other) const {
    return {static_cast<float>(*this) + static_cast<float>(other)};
}

template <typename T>
float16 float16::operator+=(const T& other) {
    return *this = *this + other;
}

template <typename T>
float16 float16::operator-(const T& other) const {
    return {static_cast<float>(*this) - static_cast<float>(other)};
}

template <typename T>
float16 float16::operator-=(const T& other) {
    return *this = *this - other;
}

template <typename T>
float16 float16::operator*(const T& other) const {
    return {static_cast<float>(*this) * static_cast<float>(other)};
}

template <typename T>
float16 float16::operator*=(const T& other) {
    return *this = *this * other;
}

template <typename T>
float16 float16::operator/(const T& other) const {
    return {static_cast<float>(*this) / static_cast<float>(other)};
}

template <typename T>
float16 float16::operator/=(const T& other) {
    return *this = *this / other;
}
}  // namespace ngraph

namespace std {
bool NGRAPH_API isnan(ngraph::float16 x);

template <>
class numeric_limits<ngraph::float16> {
public:
    static constexpr bool is_specialized = true;
    static constexpr ngraph::float16 min() noexcept {
        return ngraph::float16::from_bits(0x0200);
    }
    static constexpr ngraph::float16 max() noexcept {
        return ngraph::float16::from_bits(0x7BFF);
    }
    static constexpr ngraph::float16 lowest() noexcept {
        return ngraph::float16::from_bits(0xFBFF);
    }
    static constexpr int digits = 11;
    static constexpr int digits10 = 3;
    static constexpr bool is_signed = true;
    static constexpr bool is_integer = false;
    static constexpr bool is_exact = false;
    static constexpr int radix = 2;
    static constexpr ngraph::float16 epsilon() noexcept {
        return ngraph::float16::from_bits(0x1200);
    }
    static constexpr ngraph::float16 round_error() noexcept {
        return ngraph::float16::from_bits(0x3C00);
    }
    static constexpr int min_exponent = -13;
    static constexpr int min_exponent10 = -4;
    static constexpr int max_exponent = 16;
    static constexpr int max_exponent10 = 4;
    static constexpr bool has_infinity = true;
    static constexpr bool has_quiet_NaN = true;
    static constexpr bool has_signaling_NaN = true;
    static constexpr float_denorm_style has_denorm = denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr ngraph::float16 infinity() noexcept {
        return ngraph::float16::from_bits(0x7C00);
    }
    static constexpr ngraph::float16 quiet_NaN() noexcept {
        return ngraph::float16::from_bits(0x7FFF);
    }
    static constexpr ngraph::float16 signaling_NaN() noexcept {
        return ngraph::float16::from_bits(0x7DFF);
    }
    static constexpr ngraph::float16 denorm_min() noexcept {
        return ngraph::float16::from_bits(0);
    }
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = false;
    static constexpr bool is_modulo = false;
    static constexpr bool traps = false;
    static constexpr bool tinyness_before = false;
    static constexpr float_round_style round_style = round_to_nearest;
};
}  // namespace std
