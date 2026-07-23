# MixSan: MemTag-Enhanced Memory Safety Sanitizer

**MixSan** extends [RangeSanitizer (RSan)](https://download.vusec.net/papers/rsan_sec25.pdf) by replacing quarantine-based temporal safety with a 6-bit MemTag mechanism using Intel LAM U57. MixSan provides deterministic spatial and temporal memory error detection in C/C++ programs.

**Paper @ USENIX Security 2025**: https://download.vusec.net/papers/rsan_sec25.pdf

## Unified Check

MixSan uses a single arithmetic check for both spatial bounds and temporal (use-after-free / double-free) detection:

```
(fused_meta - (tagged_ptr + access_size)) >> 56 != 0  →  error
```

**In-bounds (pass)**: `diff` is a small positive value (remaining space ≤ object size), right-shift by 56 produces zero.

**OOB / UAF / double-free (error)**: `diff` wraps to a huge unsigned value, right-shift by 56 produces non-zero → `int3` (x86) or `brk #0x0` (Arm) trap.

This unified form is implemented identically in the compiler pass (`SafeStack.cpp`) and the runtime allocator (`common.h` MEMTAG_CHECK macro).

## Bit Layout (Intel LAM U57)

```
Bit:  63    62–57     56–47      46–41     40–0
     [sign] [MemTag] [available] [SizeTag] [offset]
```

- **MemTag** (bits 62–57): 6-bit per-allocation version for temporal safety. Derived from `rdtsc()` at `malloc`, flipped on `free` (`*meta = 0`). 63-step cycle, collision probability 1/63.
- **SizeTag** (bits 46–41): 6-bit exponent encoding the object size class. Used to reconstruct object base pointer: `obj_start = (ptr >> sc) << sc`.
- Fused metadata stored at `obj_start - 8`: encodes `(identity_tag << 57) | end_address`.

## Contents

| Path | Description |
|------|-------------|
| `examples/` | Example C programs to test basic MixSan functionality (OOB, UAF, MemTag mismatch) |
| `infra/` | Python test harness for building, running, and reporting benchmarks |
| `linker-implicit/` | Linker script and custom dynamic linker for implicit tagging |
| `llvm-project-16/` | Modified LLVM 16.0.6 — compiler instrumentation in `SafeStack.cpp` |
| `tcmalloc-implicit/` | Modified TCMalloc 2.15 with MemTag and SizeTag metadata |
| `tcmalloc-explicit/` | Modified TCMalloc 2.15 with explicit tagging (Arm TBI) |
| `tests/memtag/` | Dedicated MemTag test suite — 97 cases covering UAF, double-free, OOB, realloc |
| `97-costum-benchmark/` | Custom benchmark suite for MixSan evaluation |
| `setup.py` | Script to drive the instrumentation infrastructure |
| `env.sh` | Environment configuration (must be sourced before any command) |
| `CLAUDE.md` | Developer guide for Claude Code |

\* Compiler instrumentation: see `llvm-project-16/llvm/lib/CodeGen/SafeStack.cpp`

## Dependencies

Tested on:
- (x86) i9-13900K, Ubuntu 22.04, glibc 2.35, Linux kernel v5.15
- (Arm) Apple M2, Debian 12, glibc 2.36, Asahi Linux kernel v6.4.0

**Note**: MixSan requires Intel LAM U57 support for temporal safety. Without LAM, only spatial checks function.

```bash
sudo apt install ninja-build cmake gcc-9 autoconf2.69 bison build-essential flex texinfo libtool zlib1g-dev unzip gawk
pip3 install psutil terminaltables
```

## Quick Start

```bash
git clone https://github.com/explorerlxy/rangesanitizer.git --recurse-submodules
cd rangesanitizer
```

Edit `env.sh` and update `RSAN_TOP` with the full path where you cloned this repository.

**IMPORTANT**: Always source `env.sh` before any command:

```bash
source env.sh
```

### Build

```bash
# Build LLVM
mkdir -p $RSAN_MIX_LLVM_BUILD && cd $RSAN_MIX_LLVM_BUILD
cmake -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_ENABLE_RUNTIMES="compiler-rt" \
  -DCMAKE_BUILD_TYPE=Release -GNinja -DLLVM_PARALLEL_LINK_JOBS=1 \
  -DLLVM_TARGETS_TO_BUILD=X86 -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
  -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DCLANG_ENABLE_ARCMT=OFF \
  $RSAN_MIX_LLVM
ninja -j $(nproc)

# Build TCMalloc
cd $RSAN_MIX_TC_IMPL
autoreconf -i && ./autogen.sh || true
mkdir -p $RSAN_MIX_TC_IMPL_BUILD && cd $RSAN_MIX_TC_IMPL_BUILD
CFLAGS="-g -O2 -mbmi2" CXXFLAGS="-g -O2 -mbmi2" \
  ../tcmalloc-implicit/configure --prefix=$RSAN_MIX_TC_IMPL_BUILD
make -j $(nproc) && make install

# Build linker components
cd $RSAN_TOP/linker-implicit/globals && python3 generate_linker_script.py
cd $RSAN_TOP/linker-implicit/libdl && ./run.sh
```

### Incremental Rebuild

```bash
source env.sh

# LLVM pass (SafeStack.cpp, etc.)
cd $RSAN_MIX_LLVM_BUILD && ninja -j $(nproc)

# TCMalloc (tcmalloc.cc, common.h, etc.)
cd $RSAN_MIX_TC_IMPL_BUILD && make -j $(nproc) && make install

# Linker script
cd $RSAN_TOP/linker-implicit/globals && python3 generate_linker_script.py
```

### Test

```bash
cd examples
./test-implicit.sh
```

Expected: valid access exits 0; out-of-bounds, use-after-free, and MemTag mismatch trigger SIGTRAP (exit 133).

```
Compiling first test program...
Done.

Executing out-of-bounds testcase with valid index (expected: normal run)
0

Executing out-of-bounds testcase with invalid index (expected: crash)
./test-implicit.sh: line 26: ... Trace/breakpoint trap   ./oob 40

Compiling second test program...
Done.

Executing use-after-free testcase (expected: crash)
./test-implicit.sh: line 34: ... Trace/breakpoint trap   ./uaf

Compiling MemTag mismatch testcase...
Done.

Executing altered-MemTag testcase (expected: crash)
./test-implicit.sh: line 41: ... Trace/breakpoint trap   ./memtag_mismatch
```

## Instance Classes

Each `infra.Instance` subclass encapsulates one toolchain configuration:

| Instance | Class | Sanitizer | TCMalloc | Purpose |
|----------|-------|-----------|----------|---------|
| `baseline_O0/O2` | `Baseline` | none | baseline | Performance reference |
| `rsan-orig_O0/O2` | `RSanOrig` | SafeStack + SizeTag | implicit | Original RSan comparison |
| `mixsan_O0/O2` | `MixSan` | SafeStack + SizeTag + MemTag | implicit + MemTag | **Development target** |
| `rsan-orig-wo-check_O0/O2` | `RSanOrigWoCheck` | none (SafeStack skipped) | implicit | Isolates RSan allocator overhead |
| `mixsan-wo-check_O0/O2` | `MixSanWoCheck` | none (SafeStack skipped) | implicit + MemTag | Isolates MixSan allocator overhead |
| `mixsan-compiler-rsan-tc_O0/O2` | `MixSanCompilerRSanTC` | MixSan compiler | implicit (RSan) | Isolates compiler pass overhead |
| `asan_O0/O2` | `ASan` | ASan | system default | Reference sanitizer |

## Benchmarks

### SPEC CPU 2006

```bash
source env.sh

# Build
python3 setup.py build spec2006 baseline_O2 rsan-orig_O2 mixsan_O2 --parallel=proc --jobs=14

# Quick smoke test
python3 setup.py run spec2006 baseline_O2 --test

# Full run (3 iterations for statistical stability)
python3 setup.py run spec2006 baseline_O2 rsan-orig_O2 mixsan_O2 --iterations 3

# Report with overhead vs baseline
python3 setup.py report spec2006 results/last --overhead baseline_O2 \
    --field runtime:median maxrss:median --aggregate geomean
```

### SPEC CPU 2017

```bash
python3 setup.py build spec2017 baseline_O2 rsan-orig_O2 mixsan_O2 --parallel=proc --jobs=14
python3 setup.py run spec2017 baseline_O2 rsan-orig_O2 mixsan_O2 --iterations 3
python3 setup.py report spec2017 results/last --overhead baseline_O2 \
    --field runtime:median maxrss:median --aggregate geomean
```

### Juliet Test Suite

```bash
python3 setup.py run juliet baseline_O0 rsan-orig_O0 mixsan_O0 --build \
    --parallel=proc --parallelmax=$(nproc) \
    --cwe 121 122 124 126 127 415 416
```

Note: Juliet requires `-O0` — optimizations hide memory bugs in test cases.

### MemTag Test Suite

```bash
# Via setup.py
python3 setup.py run memtag_test baseline_O0 rsan-orig_O0 mixsan_O0 --build
python3 setup.py report memtag_test results/last --field detected missed verdict

# Direct make (faster iteration)
cd tests/memtag && make all && ./run_tests.sh
```

## Performance Optimization History

MixSan has been optimized from 6.00× overhead to 1.45× (faster than RSan's 1.52×). Measurements on SPEC2006 429.mcf (baseline=133.7s):

| Step | mcf Runtime | vs Baseline | Key Change |
|------|-----------|-------------|------------|
| Initial | 802.6s | 6.00× | 3-stage check + quarantine MemTag |
| TSC+NOT tcmalloc | 232.3s | 1.74× | `rdtsc()` replaces `*meta_ptr` read in malloc |
| Simplified SafeStack | 224.3s | 1.68× | Remove MemTag extraction from inline checks |
| Unified check | 222.1s | 1.66× | Single subtraction replaces temporal+spatial OR |
| Remove MetaMemTag | 197.5s | 1.48× | Drop redundant 3rd return value |
| **Final** | **194.1s** | **1.45×** | Re-optimized after revert |

Key insight: memory access (the `*meta_ptr` load in malloc hot path) dominated 85% of overhead before being replaced with `rdtsc()`.

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| ASan random crash | `sudo sysctl vm.mmap_rnd_bits=28` |
| Juliet runs slowly | `sudo systemctl disable --now apport.service` |
| SPEC2006 "not installed" | Run `install.sh -d <dir>` in SPEC directory |
| MixSan compiler not found | Build LLVM first (see Build section) |
| `env.sh` variables not set | Must use `source env.sh`, not `./env.sh` |
