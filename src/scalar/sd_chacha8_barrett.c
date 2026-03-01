#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t state[16];
    uint64_t counter;
} sd_chacha_scalar_state_t;

#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

static inline uint32_t barrett_mod31(uint32_t x) {
    // Barrett reduction for modulo 31
    // For 8-bit inputs (x & 0xFF), this is exact
    uint32_t q = (x * 265) >> 13;  // 265 = ceil(2^13 / 31)
    return x - (q * 31);
}

static inline void sd_chacha_quarterround_scalar(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    // Round 1
    *a += *b;
    
    // State-dependent rotation using Barrett reduction
    uint32_t rot = barrett_mod31((*c ^ *b) & 0xFF) + 1;
    *d = ROTL32(*d ^ *a, rot);
    
    // Round 2
    *c += *d;
    *b = ROTL32(*b ^ *c, 12);
    
    // Round 3
    *a += *b;
    *d = ROTL32(*d ^ *a, 8);
    
    // Round 4
    *c += *d;
    *b = ROTL32(*b ^ *c, 7);
}

#define U32TO8_LITTLE(p, v) \
    do { \
        (p)[0] = (uint8_t)((v) >>  0); \
        (p)[1] = (uint8_t)((v) >>  8); \
        (p)[2] = (uint8_t)((v) >> 16); \
        (p)[3] = (uint8_t)((v) >> 24); \
    } while (0)

// Single-block generation (scalar version)
void sd_chacha8_barrett_scalar_generate(uint8_t *out, size_t num_blocks, void *state_ptr) {
    sd_chacha_scalar_state_t *state = (sd_chacha_scalar_state_t *)state_ptr;
    
    for (size_t block = 0; block < num_blocks; block++) {
        uint32_t x[16];
        
        // Copy state to working array
        for (int i = 0; i < 16; i++) {
            x[i] = state->state[i];
        }
        
        // Set counter
        x[12] = (uint32_t)(state->counter & 0xFFFFFFFF);
        x[13] = (uint32_t)(state->counter >> 32);
        
        // Save input for final addition
        uint32_t input[16];
        for (int i = 0; i < 16; i++) {
            input[i] = x[i];
        }
        
        // Perform 8 rounds (4 double rounds)
        for (int round = 0; round < 4; round++) {
            // Column rounds
            sd_chacha_quarterround_scalar(&x[0], &x[4], &x[ 8], &x[12]);
            sd_chacha_quarterround_scalar(&x[1], &x[5], &x[ 9], &x[13]);
            sd_chacha_quarterround_scalar(&x[2], &x[6], &x[10], &x[14]);
            sd_chacha_quarterround_scalar(&x[3], &x[7], &x[11], &x[15]);
            
            // Diagonal rounds
            sd_chacha_quarterround_scalar(&x[0], &x[5], &x[10], &x[15]);
            sd_chacha_quarterround_scalar(&x[1], &x[6], &x[11], &x[12]);
            sd_chacha_quarterround_scalar(&x[2], &x[7], &x[ 8], &x[13]);
            sd_chacha_quarterround_scalar(&x[3], &x[4], &x[ 9], &x[14]);
        }
        
        // Add input to working array
        for (int i = 0; i < 16; i++) {
            x[i] += input[i];
        }
        
        // Store output
        for (int i = 0; i < 16; i++) {
            U32TO8_LITTLE(out + i * 4, x[i]);
        }
        
        out += 64;
        state->counter++;
    }
}