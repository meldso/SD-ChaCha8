// chacha_scalar.c
#include <stdint.h>
#include <string.h>

#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define U32TO8_LITTLE(p, v) \
    do { \
        (p)[0] = (uint8_t)((v) >>  0); \
        (p)[1] = (uint8_t)((v) >>  8); \
        (p)[2] = (uint8_t)((v) >> 16); \
        (p)[3] = (uint8_t)((v) >> 24); \
    } while (0)

typedef struct {
    uint32_t state[16];
    uint64_t counter;
} chacha_state_t;

static inline void chacha_quarterround(uint32_t *x, int a, int b, int c, int d) {
    x[a] += x[b]; x[d] = ROTL32(x[d] ^ x[a], 16);
    x[c] += x[d]; x[b] = ROTL32(x[b] ^ x[c], 12);
    x[a] += x[b]; x[d] = ROTL32(x[d] ^ x[a], 8);
    x[c] += x[d]; x[b] = ROTL32(x[b] ^ x[c], 7);
}

void chacha_block_scalar(uint32_t out[16], const uint32_t in[16], int rounds) {
    uint32_t x[16];
    memcpy(x, in, sizeof(x));
    
    for (int i = 0; i < rounds; i += 2) {
        // Column round
        chacha_quarterround(x, 0, 4,  8, 12);
        chacha_quarterround(x, 1, 5,  9, 13);
        chacha_quarterround(x, 2, 6, 10, 14);
        chacha_quarterround(x, 3, 7, 11, 15);
        // Diagonal round
        chacha_quarterround(x, 0, 5, 10, 15);
        chacha_quarterround(x, 1, 6, 11, 12);
        chacha_quarterround(x, 2, 7,  8, 13);
        chacha_quarterround(x, 3, 4,  9, 14);
    }
    
    for (int i = 0; i < 16; i++) {
        out[i] = x[i] + in[i];
    }
}

void chacha_scalar_generate(uint8_t *out, size_t num_blocks, void *state_ptr) {
    chacha_state_t *state = (chacha_state_t *)state_ptr;
    
    for (size_t block = 0; block < num_blocks; block++) {
        uint32_t output[16];
        state->state[12] = (uint32_t)(state->counter & 0xFFFFFFFF);
        state->state[13] = (uint32_t)(state->counter >> 32);
        
        chacha_block_scalar(output, state->state, 20); // ChaCha20
        
        for (int i = 0; i < 16; i++) {
            U32TO8_LITTLE(out + i * 4, output[i]);
        }
        
        out += 64;
        state->counter++;
    }
}

void chacha12_scalar_generate(uint8_t *out, size_t num_blocks, void *state_ptr) {
    chacha_state_t *state = (chacha_state_t *)state_ptr;
    
    for (size_t block = 0; block < num_blocks; block++) {
        uint32_t output[16];
        state->state[12] = (uint32_t)(state->counter & 0xFFFFFFFF);
        state->state[13] = (uint32_t)(state->counter >> 32);
        
        chacha_block_scalar(output, state->state, 12); // ChaCha12
        
        for (int i = 0; i < 16; i++) {
            U32TO8_LITTLE(out + i * 4, output[i]);
        }
        
        out += 64;
        state->counter++;
    }
}

void chacha8_scalar_generate(uint8_t *out, size_t num_blocks, void *state_ptr) {
    chacha_state_t *state = (chacha_state_t *)state_ptr;
    
    for (size_t block = 0; block < num_blocks; block++) {
        uint32_t output[16];
        state->state[12] = (uint32_t)(state->counter & 0xFFFFFFFF);
        state->state[13] = (uint32_t)(state->counter >> 32);
        
        chacha_block_scalar(output, state->state, 8); // ChaCha8
        
        for (int i = 0; i < 16; i++) {
            U32TO8_LITTLE(out + i * 4, output[i]);
        }
        
        out += 64;
        state->counter++;
    }
}