# MixSan MemTag Test Suite Makefile
# Usage: source ../../env.sh && make all

# ---- Compilers ----
CC_BASE  ?= $(RSAN_ORIG_C)
CXX_BASE ?= $(RSAN_ORIG_CXX)
CC_RSAN  ?= $(RSAN_ORIG_C)
CXX_RSAN ?= $(RSAN_ORIG_CXX)
CC_MIX   ?= $(RSAN_MIX_C)
CXX_MIX  ?= $(RSAN_MIX_CXX)

# ---- TCMalloc ----
TC_BASE  ?= $(RSAN_ORIG_TC_BASE_BUILD)
TC_RSAN  ?= $(RSAN_ORIG_TC_IMPL_BUILD)
TC_MIX   ?= $(RSAN_MIX_TC_IMPL_BUILD)

# ---- Linker ----
LNK_RSAN ?= $(RSAN_ORIG_LINKER_SCRIPT)
DL_RSAN  ?= $(RSAN_ORIG_DYNAMIC_LINKER)
LNK_MIX  ?= $(RSAN_MIX_LINKER_SCRIPT)
DL_MIX   ?= $(RSAN_MIX_DYNAMIC_LINKER)

# ---- Common flags ----
STD_CFLAGS   = -O0 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -g -flto=full
STD_CXXFLAGS = -O0 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -g -flto=full
STD_LDFLAGS  = -fuse-ld=lld

# ---- Baseline flags (no sanitizer, baseline tcmalloc) ----
BL_CFLAGS   = $(STD_CFLAGS)
BL_CXXFLAGS = $(STD_CXXFLAGS)
BL_LDFLAGS  = $(STD_LDFLAGS) -L$(TC_BASE)/lib/ -Wl,-rpath -Wl,$(TC_BASE)/lib/ -ltcmalloc_minimal

# ---- RSan-Orig flags (SafeStack + implicit tagging + quarantine) ----
RS_CFLAGS   = $(STD_CFLAGS) -fsanitize=safe-stack -no-pie -mbmi2
RS_CXXFLAGS = $(STD_CXXFLAGS) -fsanitize=safe-stack -no-pie -mbmi2
RS_LDFLAGS  = $(STD_LDFLAGS) -fsanitize=safe-stack -no-pie -T $(LNK_RSAN) -z max-page-size=0x1000 \
              -Wl,--dynamic-linker=$(DL_RSAN) -L$(TC_RSAN)/lib/ -Wl,-rpath -Wl,$(TC_RSAN)/lib/ -ltcmalloc_minimal

# ---- MixSan flags (SafeStack + implicit + MemTag, no quarantine) ----
MX_CFLAGS   = $(STD_CFLAGS) -fsanitize=safe-stack -no-pie -mbmi2
MX_CXXFLAGS = $(STD_CXXFLAGS) -fsanitize=safe-stack -no-pie -mbmi2
MX_LDFLAGS  = $(STD_LDFLAGS) -fsanitize=safe-stack -no-pie -T $(LNK_MIX) -z max-page-size=0x1000 \
              -Wl,--dynamic-linker=$(DL_MIX) -L$(TC_MIX)/lib/ -Wl,-rpath -Wl,$(TC_MIX)/lib/ -ltcmalloc_minimal

# ---- Thread lib ----
THREAD_LIB = -pthread

# ---- ASan compiler and flags ----
CC_ASAN  ?= /usr/bin/clang-18
CXX_ASAN ?= /usr/bin/clang++-18
AS_CFLAGS   = -O0 -g -fsanitize=address
AS_CXXFLAGS = -O0 -g -fsanitize=address
AS_LDFLAGS  = -fsanitize=address

# ---- SoftBoundCETS compiler and flags ----
CC_SB  ?= /home/hahafish/softboundcets/build/bin/clang
CXX_SB ?= /home/hahafish/softboundcets/build/bin/clang++
SB_DIR  = /home/hahafish/softboundcets/build
SB_RT_DIR = $(SB_DIR)/lib/clang/12.0.1/lib/linux

SB_CFLAGS   = -O0 -g -flto -fno-vectorize -fno-slp-vectorize -Wno-unused-command-line-argument -Wno-int-conversion -Wno-deprecated-non-prototype
SB_CXXFLAGS = -O0 -g -flto -fno-vectorize -fno-slp-vectorize -Wno-unused-command-line-argument -Wno-int-conversion -Wno-deprecated-non-prototype

# Custom non-benchmarking _invalid_locks runtime:
# __softboundcets_abort() calls abort() → SIGABRT (134) = DETECT
# Built with: -D__SOFTBOUNDCETS_PREVENT_SEGFAULTS_ON_INVALID_LOCKS=1 (no benchmarking)
SB_LDFLAGS  = -fuse-ld=$(SB_DIR)/bin/ld.lld -flto \
              -Wl,-mllvm=-load=$(SB_DIR)/lib/LLVMSoftBoundCETSLTO.so \
              -Wl,-mllvm=-softboundcets-inline-rtlib-functions \
              -Wl,-mllvm=-softboundcets-create-secondary-tries=true \
              -Wl,--disable-verify \
              -Wl,--whole-archive \
              -L$(SB_RT_DIR) -Bstatic \
              -l:libclang_rt.softboundcets_custom_inlining_invalid_locks-x86_64.a \
              -Bdynamic -Wl,--no-whole-archive \
              -lpthread -lrt -ldl

# ---- Source discovery ----
CAT_DIRS = cat1_temporal cat2_spatial

# Include generated/ subdirectories
GEN_DIRS = cat1_temporal/generated cat2_spatial/generated

ALL_DIRS = $(CAT_DIRS) $(GEN_DIRS)

C_SRCS   = $(wildcard $(addsuffix /*.c, $(ALL_DIRS)))
CXX_SRCS = $(wildcard $(addsuffix /*.cpp, $(ALL_DIRS)))

# Generate target names: <cat>/<name>_<instance>
C_BL_TARGETS   = $(C_SRCS:.c=_baseline)
C_RS_TARGETS   = $(C_SRCS:.c=_rsan)
C_MX_TARGETS   = $(C_SRCS:.c=_mixsan)
C_SB_TARGETS   = $(C_SRCS:.c=_softboundcets)
C_AS_TARGETS   = $(C_SRCS:.c=_asan)
CXX_BL_TARGETS = $(CXX_SRCS:.cpp=_baseline)
CXX_RS_TARGETS = $(CXX_SRCS:.cpp=_rsan)
CXX_MX_TARGETS = $(CXX_SRCS:.cpp=_mixsan)
CXX_SB_TARGETS = $(CXX_SRCS:.cpp=_softboundcets)
CXX_AS_TARGETS = $(CXX_SRCS:.cpp=_asan)

ALL_TARGETS = $(C_BL_TARGETS) $(C_RS_TARGETS) $(C_MX_TARGETS) $(C_SB_TARGETS) $(C_AS_TARGETS) \
              $(CXX_BL_TARGETS) $(CXX_RS_TARGETS) $(CXX_MX_TARGETS) $(CXX_SB_TARGETS) $(CXX_AS_TARGETS)

# Thread test targets for adding -lpthread
THREAD_TESTS = cat1_temporal/thread_uaf_race cat1_temporal/thread_df_race cat1_temporal/parallel_alloc_free

.PHONY: all clean baseline rsan mixsan softboundcets asan

all: $(ALL_TARGETS)

baseline: $(C_BL_TARGETS) $(CXX_BL_TARGETS)
rsan:     $(C_RS_TARGETS) $(CXX_RS_TARGETS)
mixsan:   $(C_MX_TARGETS) $(CXX_MX_TARGETS)
softboundcets: $(C_SB_TARGETS) $(CXX_SB_TARGETS)
asan:     $(C_AS_TARGETS) $(CXX_AS_TARGETS)

# ---- Build rules: C ----
%_baseline: %.c
	$(CC_BASE) $(BL_CFLAGS) -o $@ $< $(BL_LDFLAGS) $(if $(filter $(<:.c=),$(THREAD_TESTS)),$(THREAD_LIB),)

%_rsan: %.c
	$(CC_RSAN) $(RS_CFLAGS) -o $@ $< $(RS_LDFLAGS) $(if $(filter $(<:.c=),$(THREAD_TESTS)),$(THREAD_LIB),)

%_mixsan: %.c $(MX_STUB)
	$(CC_MIX) $(MX_CFLAGS) -o $@ $< $(MX_STUB) $(MX_LDFLAGS) $(if $(filter $(<:.c=),$(THREAD_TESTS)),$(THREAD_LIB),)

# ---- Build rules: C++ ----
%_baseline: %.cpp
	$(CXX_BASE) $(BL_CXXFLAGS) -o $@ $< $(BL_LDFLAGS)

%_rsan: %.cpp
	$(CXX_RSAN) $(RS_CXXFLAGS) -o $@ $< $(RS_LDFLAGS)

%_mixsan: %.cpp $(MX_STUB)
	$(CXX_MIX) $(MX_CXXFLAGS) -o $@ $< $(MX_STUB) $(MX_LDFLAGS)

# ---- Build rules: SoftBoundCETS C ----
%_softboundcets: %.c
	$(CC_SB) $(SB_CFLAGS) -o $@ $< $(SB_LDFLAGS) $(if $(filter $(<:.c=),$(THREAD_TESTS)),$(THREAD_LIB),)

# ---- Build rules: SoftBoundCETS C++ ----
%_softboundcets: %.cpp
	$(CXX_SB) $(SB_CXXFLAGS) -o $@ $< $(SB_LDFLAGS)

# ---- Build rules: ASan C ----
%_asan: %.c
	$(CC_ASAN) $(AS_CFLAGS) -o $@ $< $(AS_LDFLAGS) $(if $(filter $(<:.c=),$(THREAD_TESTS)),$(THREAD_LIB),)

# ---- Build rules: ASan C++ ----
%_asan: %.cpp
	$(CXX_ASAN) $(AS_CXXFLAGS) -o $@ $< $(AS_LDFLAGS)

# ---- Perf micro-benchmarks (cat3_performance) ----
# Split into two categories:
#   TC_ONLY: pure tcmalloc test (NO -fsanitize=safe-stack). alloc_latency + stress tests.
#   CHECK:   SafeStack check overhead test. check_overhead.
PERF_TC_SRCS = alloc_latency
PERF_CHK_SRCS = check_overhead

PERF_TC_BL = $(addsuffix _baseline, $(addprefix cat3_performance/, $(PERF_TC_SRCS)))
PERF_TC_RS = $(addsuffix _rsan,     $(addprefix cat3_performance/, $(PERF_TC_SRCS)))
PERF_TC_MX = $(addsuffix _mixsan,   $(addprefix cat3_performance/, $(PERF_TC_SRCS)))

PERF_CHK_BL = $(addsuffix _baseline, $(addprefix cat3_performance/, $(PERF_CHK_SRCS)))
PERF_CHK_RS = $(addsuffix _rsan,     $(addprefix cat3_performance/, $(PERF_CHK_SRCS)))
PERF_CHK_MX = $(addsuffix _mixsan,   $(addprefix cat3_performance/, $(PERF_CHK_SRCS)))

PERF_TARGETS = $(PERF_TC_BL) $(PERF_TC_RS) $(PERF_TC_MX) \
               $(PERF_CHK_BL) $(PERF_CHK_RS) $(PERF_CHK_MX)

PERF_THREAD = cat3_performance/alloc_latency

# Tcmalloc-only LDFLAGS: need custom dynamic linker (pld.so) and linker script
# to avoid init-order crash (tcmalloc metadata write during libstdc++ ctor).
TC_BL_LDFLAGS = $(STD_LDFLAGS) -L$(TC_BASE)/lib/ -Wl,-rpath -Wl,$(TC_BASE)/lib/ -ltcmalloc_minimal
TC_RS_LDFLAGS = $(STD_LDFLAGS) -no-pie -T $(LNK_RSAN) -z max-page-size=0x1000 -Wl,--dynamic-linker=$(DL_RSAN) -L$(TC_RSAN)/lib/ -Wl,-rpath -Wl,$(TC_RSAN)/lib/ -ltcmalloc_minimal
TC_MX_LDFLAGS = $(STD_LDFLAGS) -no-pie -T $(LNK_MIX) -z max-page-size=0x1000 -Wl,--dynamic-linker=$(DL_MIX) -L$(TC_MIX)/lib/ -Wl,-rpath -Wl,$(TC_MIX)/lib/ -ltcmalloc_minimal

perf: $(PERF_TARGETS)

# Tcmalloc-only: no SafeStack — pure library comparison.
# rsan/mix tcmalloc was NOT built with -fsanitize=safe-stack.
TC_CFLAGS = -O2 -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -g
TC_LDLIBS = -lm -pthread

$(PERF_TC_BL): cat3_performance/%_baseline: cat3_performance/%.c
	$(CC_MIX) $(TC_CFLAGS) -o $@ $< $(TC_BL_LDFLAGS) $(TC_LDLIBS)

$(PERF_TC_RS): cat3_performance/%_rsan: cat3_performance/%.c
	$(CC_MIX) $(TC_CFLAGS) -o $@ $< $(TC_RS_LDFLAGS) $(TC_LDLIBS)

$(PERF_TC_MX): cat3_performance/%_mixsan: cat3_performance/%.c
	$(CC_MIX) $(TC_CFLAGS) -o $@ $< $(TC_MX_LDFLAGS) $(TC_LDLIBS)

# ---- CHECK OVERHEAD: baseline (no safe-stack) ----
$(PERF_CHK_BL): cat3_performance/%_baseline: cat3_performance/%.c
	$(CC_BASE) $(BL_CFLAGS) -o $@ $< $(BL_LDFLAGS)

# ---- CHECK OVERHEAD: rsan (orig clang + safe-stack + rsan tcmalloc) ----
$(PERF_CHK_RS): cat3_performance/%_rsan: cat3_performance/%.c
	$(CC_RSAN) $(RS_CFLAGS) -o $@ $< $(RS_LDFLAGS)

# ---- CHECK OVERHEAD: mixsan (mix clang + safe-stack + mix tcmalloc) ----
MX_STUB = cat3_performance/sizedstack_stub.o
$(MX_STUB): cat3_performance/sizedstack_stub.c
	$(CC_MIX) -O0 -g -c -o $@ $<

$(PERF_CHK_MX): cat3_performance/%_mixsan: cat3_performance/%.c $(MX_STUB)
	$(CC_MIX) $(MX_CFLAGS) -o $@ $< $(MX_STUB) $(MX_LDFLAGS)

# Default N=10000 for small blocks, 100 for big blocks
PERF_N_SMALL ?= 10000
PERF_N_BIG   ?= 100

run-perf: perf
	@echo "=== alloc_latency: small, -i (interleaved), N=$(PERF_N_SMALL) ==="
	@for t in $(PERF_TC_BL) $(PERF_TC_RS) $(PERF_TC_MX); do \
		echo "--- $$t ---"; taskset -c 0 ./$$t -s -i -N $(PERF_N_SMALL) 2>&1 | grep -v '^#' | awk -F, '{printf "  %-10s alloc=%.0f free=%.0f total=%.0f\n", $$3, $$5, $$6, $$7}'; \
	done
	@echo ""
	@echo "=== alloc_latency: small, -d (batched), N=$(PERF_N_SMALL) ==="
	@for t in $(PERF_TC_BL) $(PERF_TC_RS) $(PERF_TC_MX); do \
		echo "--- $$t ---"; taskset -c 0 ./$$t -s -d -N $(PERF_N_SMALL) 2>&1 | grep -v '^#';
	done

run-perf-big: perf
	@echo "=== alloc_latency: big, -i (interleaved), N=$(PERF_N_BIG) ==="
	@for t in $(PERF_TC_BL) $(PERF_TC_RS) $(PERF_TC_MX); do \
		echo "--- $$t ---"; taskset -c 0 ./$$t -b -i -N $(PERF_N_BIG) 2>&1 | grep -v '^#' | awk -F, '{printf "  %-10s total=%.0f\n", $$3, $$7}'; \
	done
	@echo ""
	@echo "=== alloc_latency: big, -d (batched), N=$(PERF_N_BIG) ==="
	@for t in $(PERF_TC_BL) $(PERF_TC_RS) $(PERF_TC_MX); do \
		echo "--- $$t ---"; taskset -c 0 ./$$t -b -d -N $(PERF_N_BIG) 2>&1 | grep -v '^#';
	done

clean:
	rm -f $(ALL_TARGETS)
