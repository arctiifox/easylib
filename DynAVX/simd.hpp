#ifndef SIMD_C
#define SIMD_C

#include <immintrin.h>
#include <cmath>

class simd {
public:
    int avx_ver = 0;

    simd() {
        #ifdef __AVX2__
        
        #endif
    }
};

// ======================== 128-bit (SSE) Versions ========================
inline void simd_sqrt128(const float* input, float* output, size_t count) {
    size_t i = 0;
    for (; i + 3 < count; i += 4) {
        __m128 vin = _mm_loadu_ps(input + i);
        __m128 vout = _mm_sqrt_ps(vin);
        _mm_storeu_ps(output + i, vout);
    }
    for (; i < count; ++i) {
        output[i] = std::sqrt(input[i]);
    }
}

inline void simd_divide128(const float* base, const float* divider, float* result, size_t count) {
    size_t i = 0;
    for (; i + 3 < count; i += 4) {
        __m128 va = _mm_loadu_ps(&base[i]);
        __m128 vb = _mm_loadu_ps(&divider[i]);
        __m128 vr = _mm_div_ps(va, vb);
        _mm_storeu_ps(&result[i], vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] / divider[i];  // Fixed: division instead of multiplication
    }
}

inline __m128 log2_approx128(__m128 x) {
    __m128i xi = _mm_castps_si128(x);
    __m128 exp = _mm_cvtepi32_ps(_mm_srli_epi32(xi, 23));
    exp = _mm_sub_ps(exp, _mm_set1_ps(127.0f));

    xi = _mm_and_si128(xi, _mm_set1_epi32(0x007FFFFF));
    xi = _mm_or_si128(xi, _mm_set1_epi32(0x3f800000));
    __m128 mant = _mm_castsi128_ps(xi);

    __m128 log_mant = _mm_sub_ps(mant, _mm_set1_ps(1.0f));
    return _mm_add_ps(exp, log_mant);
}

inline __m128 exp2_approx128(__m128 x) {
    __m128i int_part = _mm_cvttps_epi32(x);
    __m128 frac = _mm_sub_ps(x, _mm_cvtepi32_ps(int_part));

    int_part = _mm_add_epi32(int_part, _mm_set1_epi32(127));
    int_part = _mm_slli_epi32(int_part, 23);
    __m128 int_result = _mm_castsi128_ps(int_part);

    __m128 frac_result = _mm_add_ps(_mm_set1_ps(1.0f), 
                                    _mm_mul_ps(frac, _mm_set1_ps(0.69314718f)));
    return _mm_mul_ps(int_result, frac_result);
}

inline void fast_pow_array128(const float* base, const float* exponent, float* out, size_t count) {
    size_t i = 0;
    for (; i + 3 < count; i += 4) {
        __m128 vx = _mm_loadu_ps(base + i);
        __m128 vy = _mm_loadu_ps(exponent + i);
        __m128 logx = log2_approx128(vx);
        __m128 prod = _mm_mul_ps(logx, vy);
        __m128 result = exp2_approx128(prod);
        _mm_storeu_ps(out + i, result);
    }
    for (; i < count; ++i) {
        float logx = std::log2(base[i]);
        out[i] = std::exp2(exponent[i] * logx);
    }
}

inline void simd_multiply128(const float* base, const float* scalar, float* result, size_t count) {
    size_t i = 0;
    for (; i + 3 < count; i += 4) {
        __m128 va = _mm_loadu_ps(&base[i]);
        __m128 vb = _mm_loadu_ps(&scalar[i]);
        __m128 vr = _mm_mul_ps(va, vb);
        _mm_storeu_ps(&result[i], vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] * scalar[i];
    }
}

// ======================== 64-bit (SSE) Versions ========================
inline void simd_sqrt64(const float* input, float* output, size_t count) {
    size_t i = 0;
    for (; i + 1 < count; i += 2) {
        __m128 vin = _mm_setr_ps(input[i], input[i+1], 1.0f, 1.0f);
        __m128 vout = _mm_sqrt_ps(vin);
        _mm_storel_pi(reinterpret_cast<__m64*>(output + i), vout);
    }
    for (; i < count; ++i) {
        output[i] = std::sqrt(input[i]);
    }
}

inline void simd_divide64(const float* base, const float* divider, float* result, size_t count) {
    size_t i = 0;
    for (; i + 1 < count; i += 2) {
        __m128 va = _mm_setr_ps(base[i], base[i+1], 1.0f, 1.0f);
        __m128 vb = _mm_setr_ps(divider[i], divider[i+1], 1.0f, 1.0f);
        __m128 vr = _mm_div_ps(va, vb);
        _mm_storel_pi(reinterpret_cast<__m64*>(result + i), vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] / divider[i];
    }
}

inline __m128 log2_approx64(__m128 x) {
    __m128i xi = _mm_castps_si128(x);
    __m128 exp = _mm_cvtepi32_ps(_mm_srli_epi32(xi, 23));
    exp = _mm_sub_ps(exp, _mm_set1_ps(127.0f));

    xi = _mm_and_si128(xi, _mm_set1_epi32(0x007FFFFF));
    xi = _mm_or_si128(xi, _mm_set1_epi32(0x3f800000));
    __m128 mant = _mm_castsi128_ps(xi);

    __m128 log_mant = _mm_sub_ps(mant, _mm_set1_ps(1.0f));
    return _mm_add_ps(exp, log_mant);
}

inline __m128 exp2_approx64(__m128 x) {
    __m128i int_part = _mm_cvttps_epi32(x);
    __m128 frac = _mm_sub_ps(x, _mm_cvtepi32_ps(int_part));

    int_part = _mm_add_epi32(int_part, _mm_set1_epi32(127));
    int_part = _mm_slli_epi32(int_part, 23);
    __m128 int_result = _mm_castsi128_ps(int_part);

    __m128 frac_result = _mm_add_ps(_mm_set1_ps(1.0f), 
                                    _mm_mul_ps(frac, _mm_set1_ps(0.69314718f)));
    return _mm_mul_ps(int_result, frac_result);
}

inline void fast_pow_array64(const float* base, const float* exponent, float* out, size_t count) {
    size_t i = 0;
    for (; i + 1 < count; i += 2) {
        __m128 vx = _mm_setr_ps(base[i], base[i+1], 1.0f, 1.0f);
        __m128 vy = _mm_setr_ps(exponent[i], exponent[i+1], 1.0f, 1.0f);
        __m128 logx = log2_approx64(vx);
        __m128 prod = _mm_mul_ps(logx, vy);
        __m128 result = exp2_approx64(prod);
        _mm_storel_pi(reinterpret_cast<__m64*>(out + i), result);
    }
    for (; i < count; ++i) {
        float logx = std::log2(base[i]);
        out[i] = std::exp2(exponent[i] * logx);
    }
}

inline void simd_multiply64(const float* base, const float* scalar, float* result, size_t count) {
    size_t i = 0;
    for (; i + 1 < count; i += 2) {
        __m128 va = _mm_setr_ps(base[i], base[i+1], 1.0f, 1.0f);
        __m128 vb = _mm_setr_ps(scalar[i], scalar[i+1], 1.0f, 1.0f);
        __m128 vr = _mm_mul_ps(va, vb);
        _mm_storel_pi(reinterpret_cast<__m64*>(result + i), vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] * scalar[i];
    }
}

// ======================== 256-bit (AVX2) Versions ========================
#ifdef __AVX2__

inline void simd_sqrt256(const float* input, float* output, size_t count) {
    size_t i = 0;
    for (; i + 7 < count; i += 8) {
        __m256 vin = _mm256_loadu_ps(input + i);
        __m256 vout = _mm256_sqrt_ps(vin);
        _mm256_storeu_ps(output + i, vout);
    }
    for (; i < count; ++i) {
        output[i] = std::sqrt(input[i]);
    }
}

inline void simd_divide256(const float* base, const float* divider, float* result, size_t count) {
    size_t i = 0;
    for (; i + 7 < count; i += 8) {
        __m256 va = _mm256_loadu_ps(base + i);
        __m256 vb = _mm256_loadu_ps(divider + i);
        __m256 vr = _mm256_div_ps(va, vb);
        _mm256_storeu_ps(result + i, vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] / divider[i];
    }
}

inline __m256 log2_approx256(__m256 x) {
    __m256i xi = _mm256_castps_si256(x);
    __m256i exp_i = _mm256_srli_epi32(xi, 23);
    __m256 exp = _mm256_cvtepi32_ps(exp_i);
    exp = _mm256_sub_ps(exp, _mm256_set1_ps(127.0f));

    xi = _mm256_and_si256(xi, _mm256_set1_epi32(0x007FFFFF));
    xi = _mm256_or_si256(xi, _mm256_set1_epi32(0x3f800000));
    __m256 mant = _mm256_castsi256_ps(xi);

    __m256 log_mant = _mm256_sub_ps(mant, _mm256_set1_ps(1.0f));
    return _mm256_add_ps(exp, log_mant);
}

inline __m256 exp2_approx256(__m256 x) {
    __m256i int_part = _mm256_cvttps_epi32(x);
    __m256 frac = _mm256_sub_ps(x, _mm256_cvtepi32_ps(int_part));

    int_part = _mm256_add_epi32(int_part, _mm256_set1_epi32(127));
    int_part = _mm256_slli_epi32(int_part, 23);
    __m256 int_result = _mm256_castsi256_ps(int_part);

    __m256 frac_result = _mm256_add_ps(_mm256_set1_ps(1.0f), 
                                      _mm256_mul_ps(frac, _mm256_set1_ps(0.69314718f)));
    return _mm256_mul_ps(int_result, frac_result);
}

inline void fast_pow_array256(const float* base, const float* exponent, float* out, size_t count) {
    size_t i = 0;
    for (; i + 7 < count; i += 8) {
        __m256 vx = _mm256_loadu_ps(base + i);
        __m256 vy = _mm256_loadu_ps(exponent + i);
        __m256 logx = log2_approx256(vx);
        __m256 prod = _mm256_mul_ps(logx, vy);
        __m256 result = exp2_approx256(prod);
        _mm256_storeu_ps(out + i, result);
    }
    for (; i < count; ++i) {
        float logx = std::log2(base[i]);
        out[i] = std::exp2(exponent[i] * logx);
    }
}

inline void simd_multiply256(const float* base, const float* scalar, float* result, size_t count) {
    size_t i = 0;
    for (; i + 7 < count; i += 8) {
        __m256 va = _mm256_loadu_ps(base + i);
        __m256 vb = _mm256_loadu_ps(scalar + i);
        __m256 vr = _mm256_mul_ps(va, vb);
        _mm256_storeu_ps(result + i, vr);
    }
    for (; i < count; ++i) {
        result[i] = base[i] * scalar[i];
    }
}

#endif // __AVX2__
#endif // SIMD_C