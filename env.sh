# Execute this script with: `source env.sh`
#
# RSAN_TOP is derived from this script's location; override it (and any other
# variable) by pre-setting it before sourcing. This script does NOT change
# the caller's working directory.

# ---- Repository root (derived, overridable) ----
_RSAN_SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd -P)"
export RSAN_TOP="${RSAN_TOP:-$_RSAN_SCRIPT_DIR}"

# ---- SPEC CPU2006 (optional external benchmark installation) ----
# Point this at your SPEC CPU2006 tree if you use the benchmark workflows.
export RSAN_SPEC2006="${RSAN_SPEC2006:-$RSAN_TOP/spec2006}"

# LLVM/Clang
export RSAN_LLVM="${RSAN_LLVM:-$RSAN_TOP/llvm-project-16/llvm}"
export RSAN_LLVM_BUILD="${RSAN_LLVM_BUILD:-$RSAN_TOP/llvm-build}"
export RSAN_C="${RSAN_C:-$RSAN_LLVM_BUILD/bin/clang}"
export RSAN_CXX="${RSAN_CXX:-$RSAN_LLVM_BUILD/bin/clang++}"

# TCMalloc
export RSAN_TC_BASE="${RSAN_TC_BASE:-$RSAN_TOP/tcmalloc-baseline}"
export RSAN_TC_BASE_BUILD="${RSAN_TC_BASE_BUILD:-$RSAN_TOP/tcmalloc-baseline-build}"
export RSAN_TC_IMPL="${RSAN_TC_IMPL:-$RSAN_TOP/tcmalloc-implicit}"
export RSAN_TC_IMPL_BUILD="${RSAN_TC_IMPL_BUILD:-$RSAN_TOP/tcmalloc-impl-build}"
export RSAN_TC_EXPL="${RSAN_TC_EXPL:-$RSAN_TOP/tcmalloc-explicit}"
export RSAN_TC_EXPL_BUILD="${RSAN_TC_EXPL_BUILD:-$RSAN_TOP/tcmalloc-expl-build}"

# Implicit tagging linking
export RSAN_LINKER_SCRIPT="${RSAN_LINKER_SCRIPT:-$RSAN_TOP/linker-implicit/globals/linkglobals.ld}"
export RSAN_DYNAMIC_LINKER="${RSAN_DYNAMIC_LINKER:-$RSAN_TOP/linker-implicit/libdl/pld.so}"

# Infra
export RSAN_INFRA="${RSAN_INFRA:-$RSAN_TOP/infra}"

# Suggested for stable benchmarking (sudo)
# echo "performance" | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
