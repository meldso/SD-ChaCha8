# SD-ChaCha8

A closed-loop ARX pseudo-random number generator with state-dependent rotations for accelerated diffusion convergence.

## Overview

SD-ChaCha8 is an 8-round Addition-Rotation-XOR (ARX) PRNG that replaces fixed rotation constants with dynamically derived, state-dependent values. This architectural innovation eliminates geometric dead zones inherent in static rotation schedules, achieving full Strict Avalanche Criterion (SAC) compliance in 2 double-rounds versus 5+ for standard ChaCha20.

## Key Features

- **Entropy-dependent routing**: Rotation magnitudes derived from internal state eliminate fixed dead zones
- **Accelerated diffusion**: 55% improvement in first-pass Hamming weight diffusion over standard ChaCha20
- **Rigorous validation**: Full pass of TestU01 BigCrush (160 tests), topological data analysis, Wigner-Dyson spectral statistics
- **Cross-platform**: Validated on both ARM64 (Apple M3) and x86-64 architectures
- **SIMD-optimized**: Barrett reduction enables efficient vectorization

## Performance (Apple M3, ARM64)

| Generator | Cycles/Byte | vs ChaCha20 | SAC Convergence |
|-----------|-------------|-------------|-----------------|
| **SD-ChaCha8 Barrett** | **1.681** | **1.67x faster** | **2 double-rounds** |
| SD-ChaCha8 original | 2.649 | 1.06x faster | 2 double-rounds |
| ChaCha12 | 2.077 | 1.35x faster | 3-4 double-rounds |
| ChaCha20 | 2.810 | 1.00x (baseline) | 5+ double-rounds |
| ChaCha8 (static) | 1.914 | 1.47x faster | Incomplete |

**Platform**: Apple M3 (ARM64), Clang -O3 -march=native, clock_gettime

## Statistical Validation

### TestU01 BigCrush
- **Status**: All 160 tests passed
- **Data processed**: ~10 TB per variant
- **Variants tested**: Original (modulo) and Barrett (optimized)
- Complete results in `/tests/bigcrush/`

### Additional Validation
- **Strict Avalanche Criterion**: Achieved by round 2 (vs round 5+ for ChaCha20)
- **Topological uniformity**: Vietoris-Rips persistent homology matches true-random baselines
- **Spectral statistics**: Eigenvalue spacing follows Wigner-Dyson GOE distribution
- **SGLD benchmark**: Posterior exploration matches OS entropy baseline

## When to Use SD-ChaCha8

### Use when:
- Pipeline requires ARX non-linearity to eliminate GF(2) hyperplane artifacts
- ChaCha20 is a throughput bottleneck
- Naively truncating to ChaCha8 degrades statistical quality
- SAC compliance needed at reduced round count

### Do NOT use when:
- Raw throughput is paramount (use xoshiro256++, PCG64 instead)
- Cryptographic security required (SD-ARX has data-dependent timing)
- Linear generators suffice for your application

## Implementation Variants

### 1. Barrett Scalar (Recommended for most users)
```c
// 1.681 cycles/byte on ARM64
// Uses Barrett reduction: (x mod 31) via multiply-shift
```

### 2. Original (Modulo-based)
```c
// 2.649 cycles/byte on ARM64
// Direct modulo operation, portable but slower
```

### 3. SIMD (Advanced)
```c
// ARM NEON / x86 AVX2 vectorization
// Requires careful handling of per-lane variable rotations
```

## Building
```bash
# Scalar implementation
gcc -O3 -march=native sd_chacha8_barrett.c -o sd_chacha8

# With benchmarks
make benchmark

# Run full test suite
make test
```

## Usage Example
```c
#include "sd_chacha8.h"

// Initialize state
sd_chacha8_state state;
sd_chacha8_init(&state, key, nonce);

// Generate random bytes
uint8_t output[64];
sd_chacha8_generate(&state, output, 64);
```

## Repository Structure
```
/src/           - Core implementations (Barrett, original, SIMD)
/tests/         - TestU01, validation scripts, statistical tests
/benchmarks/    - Performance measurement tools
/analysis/      - Diffusion analysis, TDA, spectral statistics
/docs/          - Paper, supplementary materials
```

## Citation

If you use SD-ChaCha8 in your research, please cite:
```bibtex
@article{dsouza2025sdarx,
  author = {D'Souza, Meldon},
  title = {SD-ARX: A Closed-Loop Architecture for Accelerated Diffusion in ARX Networks},
  journal = {[Under Review]},
  year = {2025}
}
```

## Important Disclaimers

**Not for cryptographic use**: Data-dependent timing of dynamic rotations introduces side-channel vulnerabilities. SD-ARX is designed for **non-cryptographic** random number generation in simulation and stochastic optimization.

**Platform-specific performance**: Throughput values are measured on Apple M3 (ARM64). x86-64 performance may differ due to different instruction latencies and SIMD widths.

**Architectural niche**: SD-ChaCha8 targets ARX-committed pipelines. For throughput-critical workloads without ARX requirements, linear generators (xoshiro256++, PCG64) are significantly faster.

## License


## Contact

**Meldon D'Souza**  
Department of Statistics and Data Science  
Christ (Deemed-to-be) University, Bengaluru, India  
Email: meldon.dsouza@mscsai.christuniversity.in

## Acknowledgments

Based on ChaCha20 by D. J. Bernstein. TestU01 validation framework by P. L'Ecuyer and R. Simard.

---

**Last updated**: March 2025  
**Status**: Research implementation, validation complete, paper under review
