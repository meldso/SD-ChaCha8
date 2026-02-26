# SD-ChaCha8

Benchmark results for SD-ChaCha8 PRNG.

## Results

Scalar: 31.55 M/sec
SSE4.2: 35.94 M/sec
Speedup: 1.14x

## Overview

SD-ChaCha8 is a modified 8-round ARX pseudo-random number generator that replaces fixed rotation constants with state-dependent values, eliminating early-round geometric dead zones.

## Key Features

- State-dependent rotations eliminate early-round geometric dead zones
- 55% improvement in single-pass Hamming weight diffusion over standard ChaCha
- Strict Avalanche Criterion (SAC) achieved by round 2
- SIMD-optimized implementations: AVX2 (1.259 cycles/byte) and LUT (2.861 cycles/byte)

## Detailed Benchmark Results

System: Intel Core i7-11800H, GCC -O3 -march=native
Compiler: GCC -O3 -march=native

| Iterations | Scalar (M/sec) | SSE4.2 (M/sec) | Speedup |
|------------|----------------|----------------|---------|
| 10M | 31.61 | 35.94 | 1.14x |
| 50M | 31.22 | 35.89 | 1.15x |
| 100M | 31.63 | 35.95 | 1.14x |
| 250M | 31.59 | 35.95 | 1.14x |
| 500M | 31.63 | 35.95 | 1.14x |
| 1000M | 31.62 | 35.95 | 1.14x |

Total iterations tested: 1.91 billion

## Implementation Variants

| Variant | Cycles/Byte | Speed vs ChaCha20 |
|---------|-------------|-------------------|
| SD-ChaCha8 AVX2 | 1.259 | 2.72x faster |
| SD-ChaCha8 LUT | 2.861 | 1.20x faster |
| ChaCha20 | 3.430 | 1.00x (baseline) |
| SD-ChaCha8 scalar | 10.846 | 0.32x |

## Important Notes

- Not cryptographic - Do not use for security applications
- Designed for ARX-committed pipelines requiring non-linear generation
- Not a replacement for throughput-optimized generators (xoshiro256++, PCG64)
- Requires AVX2 support for maximum performance

## Citation

If you use this code in your research, please cite:

## Contact

Meldon D'Souza
Department of Statistics and Data Science
Christ (Deemed-to-be) University, Bengaluru, India
Email: meldon.dsouza@mscsai.christuniversity.in
