#!/bin/bash
# MixSan MemTag Test Suite Runner
# Usage: source ../../env.sh && ./run_tests.sh [--json] [--instance baseline|rsan|mixsan]

set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Config
TIMEOUT_SEC=10
JSON_OUT=0
TARGET_INSTANCE=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --json)   JSON_OUT=1; shift ;;
        --instance) TARGET_INSTANCE="$2"; shift 2 ;;
        *) shift ;;
    esac
done

# ---- Helpers ----
detect_result() {
    # $1 = exit code
    # Returns: DETECT (133=SIGTRAP), CRASH (other non-zero), PASS (0), TIMEOUT (124)
    local ec=$1
    if [[ $ec -eq 133 ]]; then
        echo "DETECT"
    elif [[ $ec -eq 0 ]]; then
        echo "PASS"
    elif [[ $ec -eq 124 ]]; then
        echo "TIMEOUT"
    else
        echo "CRASH($ec)"
    fi
}

run_one() {
    local bin="$1"
    local name="$2"
    local instance="$3"

    if [[ ! -x "$bin" ]]; then
        echo "MISSING_BIN"
        return
    fi

    local ec=0
    timeout $TIMEOUT_SEC "$bin" > /dev/null 2>&1
    ec=$?
    detect_result "$ec"
}

# ---- Build ----
echo -e "${CYAN}=== Building all test variants ===${NC}"
make all 2>&1 | tail -5
echo ""

# ---- Gather tests ----
CATS=(cat1_temporal cat2_spatial)
INSTANCES=(baseline rsan mixsan)

if [[ -n "$TARGET_INSTANCE" ]]; then
    INSTANCES=("$TARGET_INSTANCE")
fi

declare -A RESULTS
TOTAL=0
DETECTED=0
MISSED=0

# ---- Run ----
if [[ $JSON_OUT -eq 0 ]]; then
    echo -e "${CYAN}=== Running Tests ===${NC}"
    printf "%-45s %10s %10s %10s\n" "Test" "baseline" "rsan" "mixsan"
    printf "%s\n" "$(printf '━%.0s' {1..78})"
fi

for cat in "${CATS[@]}"; do
    # Scan top-level .c files
    for src in "$cat"/*.c; do
        [[ -f "$src" ]] || continue
        name="$(basename "$src" .c)"
        row=""

        for inst in "${INSTANCES[@]}"; do
            bin="${cat}/${name}_${inst}"
            result="$(run_one "$bin" "$name" "$inst")"
            RESULTS["${name}:${inst}"]="$result"
            row="$row $(printf '%10s' "$result")"

            if [[ "$result" == "DETECT" ]]; then
                ((DETECTED++))
            elif [[ "$result" == "PASS" ]] || [[ "$result" == "MISSING_BIN" ]]; then
                :
            else
                ((MISSED++))
            fi
        done

        if [[ $JSON_OUT -eq 0 ]]; then
            printf "%-55s%s\n" "${cat}/${name}" "$row"
        fi
        ((TOTAL++))
    done

    # Scan generated/ subdirectory
    if [[ -d "$cat/generated" ]]; then
        for src in "$cat/generated"/*.c; do
            [[ -f "$src" ]] || continue
            name="$(basename "$src" .c)"
            row=""

            for inst in "${INSTANCES[@]}"; do
                bin="${cat}/generated/${name}_${inst}"
                result="$(run_one "$bin" "$name" "$inst")"
                RESULTS["${name}:${inst}"]="$result"
                row="$row $(printf '%10s' "$result")"

                if [[ "$result" == "DETECT" ]]; then
                    ((DETECTED++))
                elif [[ "$result" == "PASS" ]] || [[ "$result" == "MISSING_BIN" ]]; then
                    :
                else
                    ((MISSED++))
                fi
            done

            if [[ $JSON_OUT -eq 0 ]]; then
                printf "%-55s%s\n" "${cat}/generated/${name}" "$row"
            fi
            ((TOTAL++))
        done
    fi
done

# Skip perf tests unless explicitly requested
PERF_CAT="cat3_performance"
PERF_INSTANCES=()
if [[ -z "$TARGET_INSTANCE" ]]; then
    PERF_INSTANCES=("${INSTANCES[@]}")
else
    PERF_INSTANCES=("$TARGET_INSTANCE")
fi

for inst in "${PERF_INSTANCES[@]}"; do
    for src in "$PERF_CAT"/*.c; do
        [[ -f "$src" ]] || continue
        name="$(basename "$src" .c)"
        bin="${PERF_CAT}/${name}_${inst}"

        if [[ $JSON_OUT -eq 0 ]]; then
            if [[ -x "$bin" ]]; then
                printf "%-45s %10s\n" "${PERF_CAT}/${name}" "[manual: run with 'time $bin']"
            else
                printf "%-45s %10s\n" "${PERF_CAT}/${name}" "[not built]"
            fi
        fi
    done
done

# ---- Summary ----
echo ""
echo -e "${CYAN}=== Summary ===${NC}"
echo "Total test binaries: $(( TOTAL * ${#INSTANCES[@]} ))"
echo "Detected (SIGTRAP): $DETECTED"
echo "Passed (exit 0):    $MISSED"

# ---- Key comparison hints ----
echo ""
echo -e "${CYAN}=== Key Comparison Points ===${NC}"
echo "Check these tests for RSan vs MixSan differentiation:"
echo ""
echo "  cat1_uaf_reuse/uaf_reuse_exhaust:"
echo "    RSan -> PASS  (quarantine exhausted, victim reused -> MISS)"
echo "    MixSan -> DETECT (MemTag mismatch)"
echo ""
echo "  cat3_nonlinear_oob/oob_skip_redzone:"
echo "    RSan -> PASS  (spatial check passes through adjacent object)"
echo "    MixSan -> DETECT (MemTag mismatch as second defense)"
echo ""
echo "  cat1_uaf_reuse/df_interleaved:"
echo "    RSan -> PASS  (slot reused, bound≠0 → *meta_ptr==0 check fails → MISS)"
echo "    MixSan -> DETECT (MemTag mismatch after slot reuse)"
echo ""
echo "  cat8_cpp_temporal/virtual_call_after_free:"
echo "    RSan -> PASS (quarantine exhausted, slot reused -> MISS)"
echo "    MixSan -> DETECT (MemTag protects vtable access on freed object)"
echo ""
echo "  cat11_memtag_stats/uaf_detection_rate:"
echo "    RSan -> PASS (quarantine exhausted -> MISS)"
echo "    MixSan -> DETECT (62/63 probability per run)"
echo ""
echo "  cat12_thread/thread_uaf_race:"
echo "    RSan -> PASS (quarantine lock serializes, slot may be reused -> MISS)"
echo "    MixSan -> DETECT (MemTag mismatch, no global lock)"
echo ""
echo -e "${YELLOW}Performance comparison (manual):${NC}"
echo "  make run-perf        # alloc_latency 小对象对比"
echo "  make run-perf-big    # alloc_latency 大对象对比"

# ---- JSON output if requested ----
if [[ $JSON_OUT -eq 1 ]]; then
    echo ""
    echo "{"
    echo "  \"results\": {"
    first=true
    for key in "${!RESULTS[@]}"; do
        if [[ $first == true ]]; then first=false; else echo ","; fi
        printf "    \"%s\": \"%s\"" "$key" "${RESULTS[$key]}"
    done
    echo ""
    echo "  }"
    echo "}"
fi
