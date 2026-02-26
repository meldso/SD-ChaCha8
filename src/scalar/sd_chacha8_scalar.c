#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <smmintrin.h>  // SSE4.2

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

// SD-ChaCha8 LUT Variant
uint8_t lut[256];
void init_lut() {
    for(int i=0; i<256; i++) lut[i] = (i % 31) + 1;
}

// SSE4.2 optimized version (4-way parallel)
void sd_chacha8_sse42(__m128i *state) {
    for(int round=0; round<4; round++) {
        // Quarter round using SSE
        state[0] = _mm_add_epi32(state[0], state[1]);
        __m128i tmp = _mm_xor_si128(state[3], state[0]);
        
        __m128i rot16 = _mm_or_si128(
            _mm_slli_epi32(tmp, 16),
            _mm_srli_epi32(tmp, 16)
        );
        state[3] = rot16;
        
        state[2] = _mm_add_epi32(state[2], state[3]);
        state[1] = _mm_xor_si128(state[1], state[2]);
        
        __m128i rot12 = _mm_or_si128(
            _mm_slli_epi32(state[1], 12),
            _mm_srli_epi32(state[1], 20)
        );
        state[1] = rot12;
        
        state[0] = _mm_add_epi32(state[0], state[1]);
        state[3] = _mm_xor_si128(state[3], state[0]);
        
        __m128i rot8 = _mm_or_si128(
            _mm_slli_epi32(state[3], 8),
            _mm_srli_epi32(state[3], 24)
        );
        state[3] = rot8;
        
        state[2] = _mm_add_epi32(state[2], state[3]);
        state[1] = _mm_xor_si128(state[1], state[2]);
        
        __m128i rot7 = _mm_or_si128(
            _mm_slli_epi32(state[1], 7),
            _mm_srli_epi32(state[1], 25)
        );
        state[1] = rot7;
    }
}

// Scalar version
void sd_chacha8_scalar(uint32_t state[16]) {
    for(int round=0; round<8; round++) {
        for(int i=0; i<16; i+=4) {
            uint32_t a = state[i], b = state[i+1], c = state[i+2], d = state[i+3];
            
            a += b; d ^= a; d = ROTL(d, 16);
            c += d; b ^= c; b = ROTL(b, 12);
            a += b; d ^= a; d = ROTL(d, 8);
            c += d; b ^= c; b = ROTL(b, 7);
            
            state[i] = a; state[i+1] = b; state[i+2] = c; state[i+3] = d;
        }
    }
}

// Benchmark function with more iterations
void run_benchmark(int iterations, int use_sse) {
    uint32_t scalar_state[16];
    union {
        uint32_t u32[16];
        __m128i m128[4];
    } state;
    
    // Initialize state
    for(int i=0; i<16; i++) {
        state.u32[i] = i;
        scalar_state[i] = i;
    }
    
    init_lut();
    
    if (use_sse) {
        for(int i=0; i<iterations; i++) {
            sd_chacha8_sse42(state.m128);
        }
        // Prevent optimization
        volatile uint32_t result = state.u32[0];
    } else {
        for(int i=0; i<iterations; i++) {
            sd_chacha8_scalar(scalar_state);
        }
        volatile uint32_t result = scalar_state[0];
    }
}

int main() {
    printf("SD-ChaCha8 SSE4.2 Benchmark\n");
    printf("============================\n\n");
    
    printf("CPU Features detected at compile time:\n");
    #ifdef __SSE4_2__
    printf("SSE4.2 supported\n");
    #endif
    printf("\n");
    
    // Much larger iteration count
    int iterations = 100000000;  // 100 million
    
    printf("Running scalar version (%d iterations)...\n", iterations);
    clock_t scalar_start = clock();
    run_benchmark(iterations, 0);
    clock_t scalar_end = clock();
    
    printf("Running SSE4.2 version (%d iterations)...\n", iterations);
    clock_t sse_start = clock();
    run_benchmark(iterations, 1);
    clock_t sse_end = clock();
    
    double scalar_time = ((double)(scalar_end - scalar_start)) / CLOCKS_PER_SEC;
    double sse_time = ((double)(sse_end - sse_start)) / CLOCKS_PER_SEC;
    
    printf("\nResults (%d iterations):\n", iterations);
    printf("------------------------\n");
    printf("Scalar:  %.4f seconds (%.2f million iter/sec)\n", 
           scalar_time, (iterations / 1000000.0) / scalar_time);
    printf("SSE4.2:  %.4f seconds (%.2f million iter/sec)\n", 
           sse_time, (iterations / 1000000.0) / sse_time);
    
    if (scalar_time > 0) {
        printf("Speedup: %.2fx\n", scalar_time / sse_time);
    }
    
    return 0;
}
