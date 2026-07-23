# MixSan MemTag Test Suite

Purpose-built test suite evaluating MixSan's 6-bit LAM MemTag mechanism against
original RSan. Each test targets a specific capability gap where MixSan's
per-pointer identity tag (MemTag) provides detection beyond RSan's
quarantine+bound approach.

## Quick Start

```bash
source ../../env.sh
make rsan mixsan              # Build RSan and MixSan variants
python3 generate_tests.py     # (Re)generate parameterized tests
cd ../.. && python3 << 'EOF'  # Run from repo root (see results/memtag/run_tests.py)
```

## Test Taxonomy

### cat1_temporal — Temporal Errors

All tests follow the quarantine-exhaustion pattern:
`alloc victim → free → exhaust quarantine (10×32MB=320MB) → slot reused by dummy → UAF/DF`

RSan misses because the victim's slot is evicted from the 256MB quarantine ring
buffer and reused — RSan only tracks freed pointers within the quarantine window.
MixSan detects because the slot's new occupant gets a different MemTag — the old
pointer's MemTag no longer matches.

| Type | Mechanism | C tests | Variants |
|------|-----------|---------|----------|
| **T1 UAF** | Free → exhaust → dummy → UAF | 16 | read(6) / write(6) / thread(4) / C++(7) |
| **T3 DF** | Free → exhaust → hold → double-free | 10 | read(5) / write(3) / thread(2) / C++(2) |
| **T4 DS** | Build structure → free node → exhaust → traversal UAF | 10 | linked-list / tree / hash-table / C++(2) |
| **T5 C++** | C++ object lifetime / realloc chain | 12 | virtual / member / array / downcast / template / inheritance / realloc |

**T1 RSan DETECT exceptions (2/16)**:
- `t1_uaf_01`: Insufficient exhaustion (3×32MB=96MB < 256MB) — victim stays in
  quarantine, RSan detects. Demonstrates RSan's quarantine-window dependency.
- `t1_uaf_w01`: Redzone detection — victim=1024B, dummy=1000B in same size class.
  Dummy's redzone at [1000,1024). Write at offset 1001 falls in redzone → RSan
  bound-check triggers. Demonstrates RSan's redzone as fallback.

All other 14 T1 C tests and all T3/T4 tests: RSan MISS, MixSan DETECT.

### cat2_spatial — Spatial Errors

Tests exploit RSan's inability to distinguish *which object* a pointer should
address. When an OOB access crosses from object A's slot into object B's valid
region, RSan checks B's metadata (not A's) — the access falls within B's bound
→ passes. MixSan's MemTag in the pointer bits does not match B's MemTag → detects.

All tests use MemTag-stripped pointer arithmetic (`MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL`)
to compute physical distances, then access via the original tagged pointer.

| Source | Tests | Mechanism |
|--------|-------|-----------|
| **gen_oob_skip** | 30 | OOB from object A into adjacent object B, varying sizes (16B–8KB) and offsets |
| **gen_ringbuf** | 7 | Ring buffer overflow write into adjacent guard buffer |
| **hand-written** | 6 | Large-stride, negative-index, far-jump, MemTag-second-defense, redzone-skip, struct-field-overflow |

**Known limitation**: `struct_field_overflow` — intra-object overflow (within same
slot). Neither sanitizer detects this; documented as a shared blind spot.

### cat3_performance — Microbenchmarks

| Test | Purpose |
|------|---------|
| `alloc_latency` | Alloc/free latency: small/big blocks, interleaved/batched |
| `check_overhead` | SafeStack check overhead measurement |
| `sizedstack_stub` | SizedStack runtime stub for MixSan builds |

## RSan vs MixSan Detection Mechanism

### RSan (original)
- **Temporal**: 256MB heap quarantine ring buffer + `pthread_mutex_lock`. Tracks
  freed pointers. Detection limited to quarantine window — once evicted, metadata
  is reused by new allocation, old pointer indistinguishable from valid pointer.
- **Spatial**: Per-slot `endaddr` bound check. No pointer identity — when OOB
  lands in another valid object's region, that object's (larger) bound passes.

### MixSan (MemTag)
- **Temporal + Spatial**: 6-bit MemTag in pointer bits [62:57] (LAM U57),
  randomized per allocation via TSC. Three-stage check: SizeTag → MemTag → Spatial.
  MemTag provides object identity — even when slot metadata is reused, the old
  pointer's MemTag differs from the new occupant's MemTag (63/64 probability).
- **MemTag collision**: 1/64 probability per access. When collision occurs, both
  SizeTag and MemTag pass, detection falls back to Spatial check.

## Build Variants

Each source file compiles into two executables:

| Suffix | Instance | Compiler | TCMalloc |
|--------|----------|----------|----------|
| `_rsan` | RSan-Orig | orig clang + SafeStack | implicit tagging + quarantine |
| `_mixsan` | MixSan | mix clang + SafeStack + MemTag | implicit + MemTag (no quarantine) |

## Interpreting Results

- **exit 0** → PASS (bug not detected)
- **exit 133** (or `ec=-5` from subprocess) → DETECT (SIGTRAP)
- **exit other** → CRASH (unexpected failure, e.g. SIGSEGV=-11)

### Expected: RSan MISS, MixSan DETECT

The core metric: tests where RSan exits 0 but MixSan exits 133.
This demonstrates MixSan's MemTag providing detection where RSan's
quarantine+bound approach fails.

### Expected: Both DETECT

T1 intentionally includes 2 tests with RSan DETECT:
- Insufficient quarantine exhaustion (victim stays in quarantine window)
- Redzone overflow (write past dummy's endaddr into redzone)

These demonstrate the *conditions* under which RSan CAN detect — and by
contrast, highlight MixSan's detection independence from those conditions.

### Expected: Both PASS

`struct_field_overflow` — intra-object overflow. Both sanitizers operate
at slot granularity; sub-slot overflows within the same tagged region
are invisible to both. Documented as a shared limitation.

## Running Tests

```bash
# Build
source env.sh
make -C tests/memtag rsan mixsan -j$(nproc)

# Run from repo root
python3 results/memtag/run_tests.py

# Or run individual test
tests/memtag/cat1_temporal/generated/t1_uaf_01_rsan
echo $?  # 0 → MISS, 133 → DETECT
```

## File Structure

```
tests/memtag/
├── generate_tests.py          # Test generator (T1-T5 temporal, S1-S2 spatial)
├── Makefile
├── README.md
├── templates/                 # Jinja2-style templates for spatial tests
│   ├── oob_skip.c.tmpl
│   └── ringbuf.c.tmpl (inline)
├── cat1_temporal/
│   └── generated/             # Generated temporal tests
├── cat2_spatial/
│   ├── *.c                    # Hand-written spatial tests (6)
│   └── generated/             # Generated spatial tests (37)
└── cat3_performance/
    └── *.c                    # Performance microbenchmarks (3)
```
