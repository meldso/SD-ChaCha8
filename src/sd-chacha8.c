/*
 * SD-ChaCha8: State-Dependent ChaCha with 8 rounds
 * 
 * A closed-loop ARX PRNG with entropy-dependent rotations.
 * Based on ChaCha20 by D. J. Bernstein.
 * 
 * Author: Meldon D'Souza
 * 
 * Usage:
 *   gcc -O3 -march=native sd_chacha8.c -o sd_chacha8
 *   ./sd_chacha8
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* ========================================================================
 * CONFIGURATION
 * ======================================================================== */

#define CHACHA_ROUNDS 8
#define CHACHA_BLOCKBYTES 64

/* ========================================================================
 * STATE STRUCTURE
 * ======================================================================== */

typedef struct {
    uint32_t state[16];
    uint64_t counter;
} sd_chacha8_state;

/* ========================================================================
 * CORE PRIMITIVES
 * ======================================================================== */

#define ROTL32(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/* Barrett reduction: compute (x mod 31) for 8-bit x
 * Formula: x mod 31 = x - 31 * floor(x * 265 / 2^13)
 * where 265 = ceil(2^13 / 31)
 */
static inline uint32_t barrett_mod31(uint32_t x) {
    uint32_t q = (x * 265) >> 13;
    return x - (q * 31);
}

/* SD-ChaCha8 quarter-round with state-dependent rotation */
static inline void sd_quarterround(uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    /* First operation */
    *a += *b;
    
    /* State-dependent rotation: r = ((c XOR b) mod 31) + 1 */
    uint32_t xor_val = (*c ^ *b) & 0xFF;  /* Take low 8 bits */
    uint32_t r = barrett_mod31(xor_val) + 1;
    
    /* Apply dynamic rotation */
    *d = ROTL32(*d ^ *a, r);
    
    /* Continue with mixed rotations */
    *c += *d;
    *b = ROTL32(*b ^ *c, 12);
    
    *a += *b;
    *d = ROTL32(*d ^ *a, 8);
    
    *c += *d;
    *b = ROTL32(*b ^ *c, 7);
}

/* SD-ChaCha8 block function (8 rounds = 4 double-rounds) */
static void sd_chacha8_block(uint32_t out[16], const uint32_t in[16]) {
    uint32_t x[16];
    int i;
    
    /* Copy input to working state */
    memcpy(x, in, sizeof(uint32_t) * 16);
    
    /* 4 double-rounds (8 rounds total) */
    for (i = 0; i < 4; i++) {
        /* Column round */
        sd_quarterround(&x[0], &x[4], &x[ 8], &x[12]);
        sd_quarterround(&x[1], &x[5], &x[ 9], &x[13]);
        sd_quarterround(&x[2], &x[6], &x[10], &x[14]);
        sd_quarterround(&x[3], &x[7], &x[11], &x[15]);
        
        /* Diagonal round */
        sd_quarterround(&x[0], &x[5], &x[10], &x[15]);
        sd_quarterround(&x[1], &x[6], &x[11], &x[12]);
        sd_quarterround(&x[2], &x[7], &x[ 8], &x[13]);
        sd_quarterround(&x[3], &x[4], &x[ 9], &x[14]);
    }
    
    /* Add input to output (feedforward) */
    for (i = 0; i < 16; i++) {
        out[i] = x[i] + in[i];
    }
}

/* ========================================================================
 * PUBLIC API
 * ======================================================================== */

/* Initialize SD-ChaCha8 state with key and nonce */
void sd_chacha8_init(sd_chacha8_state *s, const uint8_t *key, const uint8_t *nonce) {
    const uint8_t sigma[16] = "expand 32-byte k";
    
    /* Constants: "expand 32-byte k" */
    s->state[0] = ((uint32_t *)sigma)[0];
    s->state[1] = ((uint32_t *)sigma)[1];
    s->state[2] = ((uint32_t *)sigma)[2];
    s->state[3] = ((uint32_t *)sigma)[3];
    
    /* Key (8 words = 32 bytes) */
    memcpy(&s->state[4], key, 32);
    
    /* Counter (initially 0) */
    s->counter = 0;
    s->state[12] = 0;
    s->state[13] = 0;
    
    /* Nonce (2 words = 8 bytes) */
    memcpy(&s->state[14], nonce, 8);
}

/* Generate random bytes */
void sd_chacha8_generate(sd_chacha8_state *s, uint8_t *out, size_t bytes) {
    uint32_t block[16];
    size_t blocks = bytes / 64;
    size_t remainder = bytes % 64;
    size_t i;
    
    /* Generate full blocks */
    for (i = 0; i < blocks; i++) {
        /* Update counter in state */
        s->state[12] = (uint32_t)(s->counter & 0xFFFFFFFF);
        s->state[13] = (uint32_t)(s->counter >> 32);
        
        /* Generate block */
        sd_chacha8_block(block, s->state);
        
        /* Copy to output */
        memcpy(out, block, 64);
        out += 64;
        s->counter++;
    }
    
    /* Handle partial block */
    if (remainder > 0) {
        s->state[12] = (uint32_t)(s->counter & 0xFFFFFFFF);
        s->state[13] = (uint32_t)(s->counter >> 32);
        
        sd_chacha8_block(block, s->state);
        memcpy(out, block, remainder);
        s->counter++;
    }
}

/* Generate a single 32-bit random number */
uint32_t sd_chacha8_random(sd_chacha8_state *s) {
    uint32_t result;
    sd_chacha8_generate(s, (uint8_t *)&result, sizeof(uint32_t));
    return result;
}

/* Generate a random double in [0, 1) */
double sd_chacha8_uniform(sd_chacha8_state *s) {
    uint64_t x;
    sd_chacha8_generate(s, (uint8_t *)&x, sizeof(uint64_t));
    return (x >> 11) * 0x1.0p-53;  /* 53-bit precision */
}

/* ========================================================================
 * UTILITY FUNCTIONS
 * ======================================================================== */

void print_hex(const uint8_t *data, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 8 == 0) printf("  ");
        else printf(" ");
    }
    printf("\n");
}

void benchmark(void) {
    sd_chacha8_state state;
    uint8_t key[32] = {0};
    uint8_t nonce[8] = {0};
    uint8_t output[1024];
    
    const size_t iterations = 1000000;
    size_t i;
    
    sd_chacha8_init(&state, key, nonce);
    
    clock_t start = clock();
    for (i = 0; i < iterations; i++) {
        sd_chacha8_generate(&state, output, sizeof(output));
    }
    clock_t end = clock();
    
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    double bytes = iterations * sizeof(output);
    double mb_per_sec = (bytes / (1024.0 * 1024.0)) / seconds;
    
    printf("Benchmark Results:\n");
    printf("  Iterations: %zu\n", iterations);
    printf("  Data generated: %.2f MB\n", bytes / (1024.0 * 1024.0));
    printf("  Time: %.3f seconds\n", seconds);
    printf("  Throughput: %.2f MB/sec\n", mb_per_sec);
}

/* ========================================================================
 * TEST VECTORS
 * ======================================================================== */

void test_vectors(void) {
    uint8_t key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    uint8_t nonce[8] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a
    };
    
    uint8_t output[128];
    sd_chacha8_state state;
    
    sd_chacha8_init(&state, key, nonce);
    sd_chacha8_generate(&state, output, 128);
    
    printf("SD-ChaCha8 Test Vector\n");
    printf("======================\n\n");
    printf("Key:\n");
    print_hex(key, 32);
    printf("\nNonce:\n");
    print_hex(nonce, 8);
    printf("\nOutput (128 bytes):\n");
    print_hex(output, 128);
    printf("\n");
}

void example_usage(void) {
    sd_chacha8_state rng;
    uint8_t key[32] = {0};  /* In practice, use a secure random key */
    uint8_t nonce[8] = {0};
    int i;
    
    /* Initialize */
    sd_chacha8_init(&rng, key, nonce);
    
    printf("Example Usage\n");
    printf("=============\n\n");
    
    /* Generate 10 random 32-bit integers */
    printf("10 random uint32_t values:\n");
    for (i = 0; i < 10; i++) {
        printf("  %10u\n", sd_chacha8_random(&rng));
    }
    
    /* Generate 10 random doubles in [0, 1) */
    printf("\n10 random doubles in [0, 1):\n");
    for (i = 0; i < 10; i++) {
        printf("  %.10f\n", sd_chacha8_uniform(&rng));
    }
    printf("\n");
}

/* ========================================================================
 * MAIN
 * ======================================================================== */

int main(void) {
    printf("SD-ChaCha8: State-Dependent ChaCha with 8 rounds\n");
    printf("=================================================\n\n");
    
    test_vectors();
    example_usage();
    benchmark();
    
    printf("\nWarning: SD-ChaCha8 is NOT cryptographically secure.\n");
    printf("Use only for non-cryptographic applications.\n");
    
    return 0;
}
