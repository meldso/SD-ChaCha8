# SD-ARX: State-Dependent Addition-Rotation-XOR Networks

A novel closed-loop architecture for ARX primitives that replaces static rotation constants with state-dependent values, fundamentally accelerating diffusion convergence.

## Overview

SD-ARX introduces a new architectural principle for Addition-Rotation-XOR (ARX) networks: **entropy-dependent routing**. Rather than applying fixed rotation schedules, SD-ARX derives rotation magnitudes dynamically from the evolving internal state, creating a feedback mechanism that eliminates geometric dead zones and dramatically accelerates diffusion.

This repository contains the reference implementation instantiated as **SD-ChaCha8**, an 8-round PRNG demonstrating the SD-ARX principle.

## The SD-ARX Architecture

### Core Innovation

Traditional ARX designs use static rotation constants:
```
d[i+1] = (d[i] XOR a[i+1]) <<< R_static
```

SD-ARX couples rotation to state entropy:
```
r = R(state)                              // Derive rotation from state
d[i+1] = (d[i] XOR a[i+1]) <<< r         // Apply state-dependent rotation
```

### Architectural Advantages

- **Eliminates dead zones**: Static schedules create geometric isolation; state-dependent rotations eliminate fixed unreachable positions
- **Accelerates diffusion**: ~14.3% improvement in first-pass Hamming weight diffusion
- **Maintains bijectivity**: Proven invertible when R depends only on unchanged state words
- **Generalizable**: Applies to any ARX primitive (ChaCha, Salsa20, NORX, BLAKE)

## Performance (Apple M3, ARM64)

| Generator | Cycles/Byte | vs ChaCha20 | SAC Convergence |
|-----------|-------------|-------------|-----------------|
| **SD-ChaCha8 Barrett** | **1.681** | **1.67x faster** | **2 double-rounds** |
| SD-ChaCha8 (modulo) | 2.649 | 1.06x faster | 2 double-rounds |
| ChaCha20 | 2.810 | 1.00x (baseline) | 2 double-rounds |
| ChaCha12 | 2.077 | 1.35x faster | 3-4 double-rounds |
| ChaCha8 | 1.914 | 1.47x faster | Incomplete |

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

Apache License

Copyright (c) 2026 Meldon D'Souza

Version 2.0, January 2004
http://www.apache.org/licenses/
TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION
1. Definitions.
"License" shall mean the terms and conditions for use, reproduction, and distribution as defined by Sections 1 through 9 of this document.
"Licensor" shall mean the copyright owner or entity authorized by the copyright owner that is granting the License.
"Legal Entity" shall mean the union of the acting entity and all other entities that control, are controlled by, or are under common control with that entity. For the purposes of this definition, "control" means (i) the power, direct or indirect, to cause the direction or management of such entity, whether by contract or otherwise, or (ii) ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial ownership of such entity.
"You" (or "Your") shall mean an individual or Legal Entity exercising permissions granted by this License.
"Source" form shall mean the preferred form for making modifications, including but not limited to software source code, documentation source, and configuration files.
"Object" form shall mean any form resulting from mechanical transformation or translation of a Source form, including but not limited to compiled object code, generated documentation, and conversions to other media types.
"Work" shall mean the work of authorship, whether in Source or Object form, made available under the License, as indicated by a copyright notice that is included in or attached to the work (an example is provided in the Appendix below).
"Derivative Works" shall mean any work, whether in Source or Object form, that is based on (or derived from) the Work and for which the editorial revisions, annotations, elaborations, or other modifications represent, as a whole, an original work of authorship. For the purposes of this License, Derivative Works shall not include works that remain separable from, or merely link (or bind by name) to the interfaces of, the Work and Derivative Works thereof.
"Contribution" shall mean any work of authorship, including the original version of the Work and any modifications or additions to that Work or Derivative Works thereof, that is intentionally submitted to Licensor for inclusion in the Work by the copyright owner or by an individual or Legal Entity authorized to submit on behalf of the copyright owner. For the purposes of this definition, "submitted" means any form of electronic, verbal, or written communication sent to the Licensor or its representatives, including but not limited to communication on electronic mailing lists, source code control systems, and issue tracking systems that are managed by, or on behalf of, the Licensor for the purpose of discussing and improving the Work, but excluding communication that is conspicuously marked or otherwise designated in writing by the copyright owner as "Not a Contribution."
"Contributor" shall mean Licensor and any individual or Legal Entity on behalf of whom a Contribution has been received by Licensor and subsequently incorporated within the Work.
2. Grant of Copyright License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable copyright license to reproduce, prepare Derivative Works of, publicly display, publicly perform, sublicense, and distribute the Work and such Derivative Works in Source or Object form.
3. Grant of Patent License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable (except as stated in this section) patent license to make, have made, use, offer to sell, sell, import, and otherwise transfer the Work, where such license applies only to those patent claims licensable by such Contributor that are necessarily infringed by their Contribution(s) alone or by combination of their Contribution(s) with the Work to which such Contribution(s) was submitted. If You institute patent litigation against any entity (including a cross-claim or counterclaim in a lawsuit) alleging that the Work or a Contribution incorporated within the Work constitutes direct or contributory patent infringement, then any patent licenses granted to You under this License for that Work shall terminate as of the date such litigation is filed.
4. Redistribution. You may reproduce and distribute copies of the Work or Derivative Works thereof in any medium, with or without modifications, and in Source or Object form, provided that You meet the following conditions:
• (a) You must give any other recipients of the Work or Derivative Works a copy of this License; and
• (b) You must cause any modified files to carry prominent notices stating that You changed the files; and
• (c) You must retain, in the Source form of any Derivative Works that You distribute, all copyright, patent, trademark, and attribution notices from the Source form of the Work, excluding those notices that do not pertain to any part of the Derivative Works; and
• (d) If the Work includes a "NOTICE" text file as part of its distribution, then any Derivative Works that You distribute must include a readable copy of the attribution notices contained within such NOTICE file, excluding those notices that do not pertain to any part of the Derivative Works, in at least one of the following places: within a NOTICE text file distributed as part of the Derivative Works; within the Source form or documentation, if provided along with the Derivative Works; or, within a display generated by the Derivative Works, if and wherever such third-party notices normally appear. The contents of the NOTICE file are for informational purposes only and do not modify the License. You may add Your own attribution notices within Derivative Works that You distribute, alongside or as an addendum to the NOTICE text from the Work, provided that such additional attribution notices cannot be construed as modifying the License.
You may add Your own copyright statement to Your modifications and may provide additional or different license terms and conditions for use, reproduction, or distribution of Your modifications, or for any such Derivative Works as a whole, provided Your use, reproduction, and distribution of the Work otherwise complies with the conditions stated in this License.
5. Submission of Contributions. Unless You explicitly state otherwise, any Contribution intentionally submitted for inclusion in the Work by You to the Licensor shall be under the terms and conditions of this License, without any additional terms or conditions. Notwithstanding the above, nothing herein shall supersede or modify the terms of any separate license agreement you may have executed with Licensor regarding such Contributions.
6. Trademarks. This License does not grant permission to use the trade names, trademarks, service marks, or product names of the Licensor, except as required for reasonable and customary use in describing the origin of the Work and reproducing the content of the NOTICE file.
7. Disclaimer of Warranty. Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any risks associated with Your exercise of permissions under this License.
8. Limitation of Liability. In no event and under no legal theory, whether in tort (including negligence), contract, or otherwise, unless required by applicable law (such as deliberate and grossly negligent acts) or agreed to in writing, shall any Contributor be liable to You for damages, including any direct, indirect, special, incidental, or consequential damages of any character arising as a result of this License or out of the use or inability to use the Work (including but not limited to damages for loss of goodwill, work stoppage, computer failure or malfunction, or any and all other commercial damages or losses), even if such Contributor has been advised of the possibility of such damages.
9. Accepting Warranty or Additional Liability. While redistributing the Work or Derivative Works thereof, You may choose to offer, and charge a fee for, acceptance of support, warranty, indemnity, or other liability obligations and/or rights consistent with this License. However, in accepting such obligations, You may act only on Your own behalf and on Your sole responsibility, not on behalf of any other Contributor, and only if You agree to indemnify, defend, and hold each Contributor harmless for any liability incurred by, or claims asserted against, such Contributor by reason of your accepting any such warranty or additional liability.


## Contact

**Meldon D'Souza**  
Department of Statistics and Data Science  
Christ (Deemed-to-be) University, Bengaluru, India  
Email: meldon.dsouza@mscsai.christuniversity.in

## Acknowledgments

Based on ChaCha20 by D. J. Bernstein. TestU01 validation framework by P. L'Ecuyer and R. Simard.

---

**Last updated**: March 2026  
**Status**: Research implementation, validation complete, paper under review
