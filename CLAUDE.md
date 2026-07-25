# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with the SymAFL RSan toolchain.

## Scope

RSan provides SymAFL's modified LLVM 16 CodeGen pipeline, RangeSanitizer memory instrumentation, tagged TCMalloc implementations, and linker/loader support. The actual SymAFL compiler pass lives in this subtree; the shared symbolic runtime lives in [`../symcc/`](../symcc/) and AFL++ PCBT logic in [`../AFLplusplus/`](../AFLplusplus/).

## Architecture and Source Ownership

| Responsibility | Files |
|---|---|
| RSan SafeStack instrumentation and pointer/tag checks | `llvm-project-16/llvm/lib/CodeGen/SafeStack.cpp` |
| SymAFL module setup and instrumentation eligibility | `llvm-project-16/llvm/lib/CodeGen/SymCC/Pass.cpp` |
| Per-instruction symbolic IR generation and intercepted calls | `llvm-project-16/llvm/lib/CodeGen/SymCC/Symbolizer.cpp` |
| Compiler-side runtime declarations | `llvm-project-16/llvm/lib/CodeGen/SymCC/Runtime.cpp`, `Runtime.h` |
| CodeGen registration and pass order | `llvm-project-16/llvm/lib/CodeGen/TargetPassConfig.cpp`, `CMakeLists.txt` |
| x86 implicit-tag allocator, metadata, quarantine | `tcmalloc-implicit/src/common.h`, `tcmalloc.cc`, `page_heap.cc` |
| Arm explicit-tag allocator | `tcmalloc-explicit/` |
| x86 linker script generator and custom dynamic linker | `linker-implicit/globals/generate_linker_script.py`, `linker-implicit/libdl/run.sh` |
| OOB/UAF smoke examples | `examples/oob.c`, `examples/uaf.c`, `examples/test-implicit.sh` |

### Required Compiler Contract

The CodeGen pipeline order is non-negotiable:

```
SafeStackLegacyPass → SymbolizeLegacyPass → StackProtectorPass
```

SymCC must run **after** SafeStack so it observes RSan-generated memory-safety branches. It is disabled by default and is enabled with:

- Full LTO link: `-flto=full -Wl,-plugin-opt=-enable-symcc`
- Non-LTO CodeGen experiments: `-mllvm -enable-symcc`

SymAFL relies on the Full LTO route. Per-TU `-flto=full -c` does not invoke CodeGen and therefore does not run these passes.

`Pass.cpp::shouldInstrument()` deliberately skips declarations, `__noinstrument_*` (including mangled names), `__sym_ctor*`, `__safestack_init`, `__interceptor_*`, and functions with `DisableSanitizerInstrumentation`. Preserve these exclusions. `Symbolizer::handleFunctionCall()` redirects intercepted calls per call site; do not reintroduce global function renaming, which would affect skipped SafeStack runtime functions.

`SafeStack.cpp` uses generic `LShr` + `Shl` in critical x86 tag-extraction paths instead of the BMI2 BZHI intrinsic, preventing prior instruction-selection failures. This compiler change does **not** mean the x86 implicit allocator is BMI2-free: `tcmalloc-implicit` may still use `_bzhi_u64` and is built with its own architecture requirements.

## Environment and Build

`/home/hahafish/SymAFL` is a symbolic link to the active checkout at `/media/hahafish/Data/ForUbuntu/SymAFL`; the historical-looking paths in `env.sh` and root helper scripts resolve to this same repository. `env.sh` still changes the current directory when sourced.

```bash
export SYMAFL_ROOT=/media/hahafish/Data/ForUbuntu/SymAFL
cd "$SYMAFL_ROOT/RSan"
source env.sh                 # Sets RSAN_LLVM_BUILD and related variables; changes cwd.
cd "$RSAN_LLVM_BUILD"
ninja LLVMCodeGen clang lld   # Incremental modified LLVM build
```

Verify that the integrated SymCC pass is linked:

```bash
nm "$RSAN_LLVM_BUILD/lib/libLLVMCodeGen.a" | grep createSymCC
```

For a full bootstrap, after correcting `env.sh` for the current checkout:

```bash
cd "$SYMAFL_ROOT/RSan"
source env.sh
./install-all.sh
```

`install-all.sh` chooses the platform setup: x86_64 builds the implicit-tagging allocator and linker components; Arm builds the explicit-tagging allocator under TBI assumptions. SymAFL's supplied helper flow targets x86_64 implicit tagging.

## Build Targets and Smoke Tests

The root helper encapsulates the target link contract. It always selects Full LTO; `--symcc` additionally links the SymCC runtime, while `--no-rsan` omits SafeStack and RSan allocator/linker flags.

```bash
export SYMAFL_ROOT=/media/hahafish/Data/ForUbuntu/SymAFL
# Audit/update the helper's legacy absolute paths first.
source "$SYMAFL_ROOT/symafl-env.sh"
symafl-build --symcc target.c -o target
```

For RSan-only smoke tests, use the examples appropriate to the architecture:

```bash
cd "$SYMAFL_ROOT/RSan/examples"
./test-implicit.sh  # x86: valid access succeeds; invalid OOB/UAF triggers SIGTRAP
# ./test-explicit.sh # Arm explicit-tagging configuration
```

After SymCC changes, verify both the pass and output instrumentation:

```bash
nm "$RSAN_LLVM_BUILD/lib/libLLVMCodeGen.a" | grep createSymCC
nm target | grep '_sym_notify_basic_block'
nm target | grep '__afl_area_ptr'
```

Existing benchmark entry points are in `setup.py`; use its documented `spec2006` and `juliet` workflows only after building their respective infrastructure/dependencies.

## Pitfalls

- **LTO is mandatory for the integrated design.** Omitting `-flto=full` yields an uninstrumented target even if the pass flag is present.
- **`env.sh` changes the current directory** and contains checkout-specific variables. Do not assume it is relocation-safe.
- **The custom linker script, `pld.so`, and implicit TCMalloc form one contract.** Do not mix them with a stock allocator/linker for x86 implicit-tag targets.
- **LLVM builds are memory-intensive.** Reduce parallelism if the build is terminated by OOM.
- **Keep non-instrumented runtime stubs out of the integrated compiler path.** Compile them with GCC where required to avoid recursive symbolic instrumentation.
- **AArch64 explicit RSan exists independently, but is not the SymAFL helper-script path.** Keep architecture-specific claims and tests separate.

For QSYM runtime and `.pct` persistence changes, continue with [`../symcc/CLAUDE.md`](../symcc/CLAUDE.md). For PCBT screening behavior, continue with [`../AFLplusplus/CLAUDE.md`](../AFLplusplus/CLAUDE.md).
