#!/usr/bin/env python3
"""Generate MemTag test cases — v3: nuanced endaddr-based T1, removed T2.

Temporal types (cat1_temporal): 4 types = 56 generated + 6 hand-written
  T1: Basic UAF (20 tests) — same-sizeclass different-endaddr pairs
      3 RSan DETECT + 17 MISS — demonstrates endaddr=bound semantics
  T3: Double-free after slot reuse (12 tests)
  T4: Data structure traversal UAF (12 tests)
  T5: C++ object lifetime / realloc chain (12 tests)

Spatial types (cat2_spatial): unchanged (37 generated)
"""

import os, sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
TEMPLATE_DIR = os.path.join(SCRIPT_DIR, "templates")
os.chdir(SCRIPT_DIR)

def render(tmpl_path, vars_dict):
    with open(tmpl_path) as f:
        content = f.read()
    for k, v in vars_dict.items():
        content = content.replace("{{%s}}" % k, str(v))
    return content

def write_test(cat, name, content, ext=".c"):
    path = os.path.join(SCRIPT_DIR, cat, name + ext)
    with open(path, "w") as f:
        f.write(content)
    return path

def _inc():
    return "#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n#include <unistd.h>"

def _inc_cpp():
    return "#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n#include <unistd.h>"

# 10 x 32MB = 320MB, well above 256MB quarantine ring buffer
EXHAUST = """    for (int _i = 0; _i < 10; _i++) {
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) { _t[0] = (char)_i; free((void*)_t); }
    }"""

# ============================================================
# T1: UAF with same-sizeclass different-endaddr dummy (20 tests)
# ============================================================
# Core insight:
#   RSan's detection depends on the quarantine window. When quarantine is fully
#   exhausted (10x32MB=320MB > 256MB ring), the victim is evicted and the slot
#   can be reused by dummy — RSan MISS. When exhaustion is insufficient (fewer
#   chunks), the victim stays in quarantine metadata — RSan DETECT.
#
#   MixSan uses MemTag (per-allocation identity tag in pointer bits [62:57]).
#   When the slot is reused, the new allocation gets a different MemTag.
#   Old pointer carries the original MemTag → mismatch → DETECT (63/64).
#   This works REGARDLESS of quarantine state — MixSan detects even when
#   RSan's quarantine window has closed.
#
#   Test design:
#     DETECT (3):  insufficient exhaustion → victim still in quarantine
#     MISS (9):    full exhaustion → victim evicted, slot reused

def gen_t1():
    """20 tests — read(6) / write(6) / thread(4) / C++(4).
    2 RSan DETECT: read (insufficient exhaust) + write (redzone).
    """
    tests = []

    # --- Read (6): 1 DETECT + 5 MISS ---
    # DETECT: insufficient exhaustion (96MB < 256MB quarantine)
    read_cfgs = [
        (64,  64,  3,  0,   "DETECT"),  # victim stays in quarantine
        (32,  32, 10,  0,   "MISS"),
        (96,  96, 10,  0,   "MISS"),
        (128, 128, 10,  0,   "MISS"),
        (512, 512, 10,  0,   "MISS"),
        (2048,2048,10,  0,   "MISS"),
    ]
    for i, (vsz, dsz, nch, off, expect) in enumerate(read_cfgs, 1):
        name = f"t1_uaf_{i:02d}"
        write_test("cat1_temporal/generated", name,
                   _t1_body(vsz, dsz, nch, off, expect, "read"))
        tests.append(name)

    # --- Write (6): 1 DETECT + 5 MISS ---
    # DETECT: redzone — dummy(1000)<victim(1024), p[1001] in redzone
    write_cfgs = [
        (1024,1000,10,1001,"DETECT"),  # write into redzone
        (32,  32, 10,  0,   "MISS"),
        (64,  64, 10,  0,   "MISS"),
        (128, 128, 10,  0,   "MISS"),
        (256, 256, 10,  0,   "MISS"),
        (1024,1024,10,  0,   "MISS"),
    ]
    for i, (vsz, dsz, nch, off, expect) in enumerate(write_cfgs, 1):
        name = f"t1_uaf_w{i:02d}"
        write_test("cat1_temporal/generated", name,
                   _t1_body(vsz, dsz, nch, off, expect, "write"))
        tests.append(name)

    # --- Thread (4): all MISS ---
    for i, (vsz, dsz) in enumerate([(64, 64), (128, 128), (256, 256), (512, 512)], 1):
        name = f"t1_uaf_t{i:02d}"
        write_test("cat1_temporal/generated", name, _t1_thread(vsz, dsz))
        tests.append(name)

    # --- C++ (7): 3 simple + 3 patterned ---
    for i, (vsz, dsz) in enumerate([(32, 32), (64, 64), (128, 128), (256, 256)], 1):
        name = f"t1_uaf_c{i:02d}"
        write_test("cat1_temporal/generated", name,
                   _t1_cpp(vsz, dsz, 10, "MISS"), ext=".cpp")
        tests.append(name)
    # C++ specific patterns: member func, array mismatch, vector iterator
    name = "t1_uaf_c05"
    write_test("cat1_temporal/generated", name, _t1_cpp_member_func(), ext=".cpp")
    tests.append(name)
    name = "t1_uaf_c06"
    write_test("cat1_temporal/generated", name, _t1_cpp_array_mismatch(), ext=".cpp")
    tests.append(name)
    name = "t1_uaf_c07"
    write_test("cat1_temporal/generated", name, _t1_cpp_vec_iterator(), ext=".cpp")
    tests.append(name)

    return tests


def _t1_body(vsz, dsz, nch, off, expect, variant):
    """Generate T1 test: alloc victim → free → exhaust → alloc dummy → UAF.

    Parameters:
      vsz, dsz: victim and dummy allocation sizes (same size class)
      nch: number of 32MB exhaustion chunks
      off: UAF access offset (0 for base, or >dsz for redzone DETECT)
      expect: "DETECT" or "MISS"
      variant: "read" or "write"
    """
    exhaust_total = nch * 32
    if off > 0:
        verdict_note = (f"// RSan: offset {off} >= dummy endaddr({dsz}) → falls in redzone → DETECT"
                        if expect == "DETECT" else
                        f"// RSan: offset {off} < dummy endaddr({dsz}) → within bound → MISS")
    elif nch < 8:
        verdict_note = f"// {exhaust_total}MB < 256MB quarantine → victim stays → RSan DETECT"
    else:
        verdict_note = f"// {exhaust_total}MB > 256MB quarantine → victim evicted → RSan MISS"

    if variant == "write":
        access = f"p[{off}] = 0xCD; printf(\"d[0]=%02x d[{off}]=%02x\\\\n\", (unsigned char)d[0], {off}, (unsigned char)d[{off}]);"
    else:
        access = f"volatile char c = p[{off}]; printf(\"p[%d]=%02x d[0]=%02x\\\\n\", {off}, (unsigned char)c, (unsigned char)d[0]);"

    return f"""/* T1 UAF ({variant}): v={vsz}B d={dsz}B off={off} exhaust={nch}x32MB → RSan {expect} */
{_inc()}
int main(void) {{
    // Step 1: allocate victim
    char *p = (char*)malloc({vsz});
    if (!p) return 1;
    memset(p, 0xAB, {min(vsz,256)});
    // Step 2: free → enters quarantine
    free(p);
    // Step 3: exhaust quarantine ({nch} x 32MB = {exhaust_total}MB)
    for (int _i = 0; _i < {nch}; _i++) {{
        volatile char *_t = (char*)malloc(32U*1024*1024);
        if (_t) {{ _t[0] = (char)_i; free((void*)_t); }}
    }}
    // Step 4: dummy allocation — same size class, possibly different endaddr
    char *d = (char*)malloc({dsz});
    if (!d) return 1;
    memset(d, 0xCC, {min(dsz,256)});
    // Step 5: UAF via dangling pointer at offset {off}
    {access}
    {verdict_note}
    _exit(0);
}}
"""


def _t1_thread(vsz, dsz):
    return f"""/* T1 UAF (thread): victim={vsz}B dummy={dsz}B → RSan MISS */
{_inc()}
#include <pthread.h>
static char *g = NULL; static volatile int r = 0;
void *wrk(void *a) {{ (void)a; while(!r); volatile char c=g[0]; printf("g[0]=%02x\\n",(unsigned char)c); return NULL; }}
int main(void) {{
    g = (char*)malloc({vsz}); if(!g) return 1; memset(g,0xAB,{min(vsz,256)});
    free(g);
    pthread_t th; pthread_create(&th,NULL,wrk,NULL);
{EXHAUST}
    char *d = (char*)malloc({dsz}); d[0]='D';
    r=1; pthread_join(th,NULL); _exit(0);
}}
"""


def _t1_cpp(vsz, dsz, nch, expect):
    sz = max(8, vsz - 8)
    return f"""/* T1 UAF (C++): victim={vsz}B dummy={dsz}B exhaust={nch}x32MB → RSan {expect} */
{_inc_cpp()}
class W {{ public: char data[{sz}]; int id; W(int i):id(i){{}} int g(){{return id;}} }};
int main(void) {{
    W *w = new W(42); printf("id=%d\\n", w->g()); delete w;
    for (int _i = 0; _i < {nch}; _i++) {{
        volatile char *_t = new char[32U*1024*1024];
        if (_t) {{ _t[0] = (char)_i; delete[] _t; }}
    }}
    char *d = new char[{dsz}]; d[0] = 'D';
    printf("stale=%d\\n", w->g());  // virtual call through dangling ptr
    _exit(0);
}}
"""


def _t1_cpp_member_func():
    """Non-virtual member function accessing this->data after delete + exhaustion."""
    return """/* T1 C++ member func UAF: non-virtual method on dangling this */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
class Logger {
    char buf[56]; int wr;
public:
    Logger() : wr(0) { buf[0] = 0; }
    void log(const char *m) { int n = strlen(m); if (wr+n < 55) { strcpy(buf+wr, m); wr += n; } }
    void flush() { printf("LOG[%d]: %s\\n", wr, buf); wr = 0; }
};
int main(void) {
    Logger *log = new Logger(); log->log("hello"); log->flush(); delete log;
    for (int i = 0; i < 10; i++) { volatile char *t = new char[32U*1024*1024]; if (t) { t[0] = (char)i; delete[] t; } }
    char *d = new char[64]; d[0] = 'D';
    log->log("uaf!"); log->flush();  // UAF: this->buf access via dangling pointer
    _exit(0);
}
"""

def _t1_cpp_array_mismatch():
    """new[]/delete mismatch → UAF after exhaustion."""
    return """/* T1 C++ array mismatch: new[]/delete + UAF after exhaustion */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Rec { int id; char name[60]; Rec() : id(0) { name[0] = 0; } };
int main(void) {
    Rec *arr = new Rec[5];
    for (int i = 0; i < 5; i++) arr[i].id = i + 100;
    printf("arr[2].id=%d\\n", arr[2].id);
    delete arr;  // mismatch: should be delete[]
    for (int i = 0; i < 10; i++) { volatile char *t = new char[32U*1024*1024]; if (t) { t[0] = (char)i; delete[] t; } }
    char *d = new char[5 * 64]; d[0] = 'D';
    printf("stale=%d\\n", arr[2].id);  // UAF
    _exit(0);
}
"""

def _t1_cpp_vec_iterator():
    """Vector realloc frees old data → iterator becomes dangling → exhaustion → UAF."""
    return """/* T1 C++ vector iterator UAF: realloc frees old data, exhaustion, UAF via iterator */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct Vec { int *d; size_t cap, sz; Vec() : d(NULL), cap(0), sz(0) {} };
void vp(Vec *v, int x) {
    if (v->sz >= v->cap) { size_t nc = v->cap ? v->cap*2 : 16; int *nd = (int*)malloc(nc*4); for (size_t i=0;i<v->sz;i++) nd[i]=v->d[i]; free(v->d); v->d=nd; v->cap=nc; }
    v->d[v->sz++] = x;
}
int main(void) {
    Vec v; vp(&v,10); vp(&v,20); vp(&v,30); vp(&v,40); vp(&v,50); vp(&v,60); vp(&v,70); vp(&v,80);
    int *it = v.d;  // save iterator before realloc
    printf("*it=%d\\n", *it);
    for (int i = 0; i < 100; i++) vp(&v, i);  // force realloc → old d freed
    for (int i = 0; i < 10; i++) { volatile char *t = (char*)malloc(32U*1024*1024); if (t) { t[0] = (char)i; free((void*)t); } }
    int *dummy = (int*)malloc(64); dummy[0] = 0xDD;
    printf("stale=%d\\n", *it);  // UAF via stale iterator
    _exit(0);
}
"""

# ============================================================
# T3: Double-free after slot reuse (12 tests)
# ============================================================

def gen_t3():
    tests = []
    cycles = [1, 5, 15, 30, 49]

    for i, n in enumerate(cycles):
        name = f"t3_df_{i+1:02d}"
        write_test("cat1_temporal/generated", name, _t3("read", n))
        tests.append(name)
    for i, n in enumerate(cycles[:3]):
        name = f"t3_df_w{i+1:02d}"
        write_test("cat1_temporal/generated", name, _t3("write", n))
        tests.append(name)
    for i, n in enumerate(cycles[:2]):
        name = f"t3_df_t{i+1:02d}"
        write_test("cat1_temporal/generated", name, _t3("thread", n))
        tests.append(name)
    for i, n in enumerate(cycles[:2]):
        name = f"t3_df_c{i+1:02d}"
        write_test("cat1_temporal/generated", name, _t3("cpp", n), ext=".cpp")
        tests.append(name)
    return tests


def _t3(v, n):
    sz = 64
    if v == "read":
        return f"""/* T3 DF ({n} holds before DF) */
{_inc()}
int main(void) {{
    char *p = (char*)malloc({sz}); if(!p) return 1; memset(p,0xAB,{sz});
    free(p);
{EXHAUST}
    char *_hold[{n}];
    for (int _i = 0; _i < {n}; _i++) {{ _hold[_i]=(char*)malloc({sz}); if(_hold[_i]) _hold[_i][0]=(char)_i; }}
    free(p); printf("done\\n"); _exit(0);
}}
"""
    elif v == "write":
        return f"""/* T3 DF write ({n} holds) */
{_inc()}
int main(void) {{
    char *p = (char*)malloc({sz}); if(!p) return 1; memset(p,0xAB,{sz});
    free(p);
{EXHAUST}
    char *_hold[{n}];
    for (int _i = 0; _i < {n}; _i++) {{ _hold[_i]=(char*)malloc({sz}); if(_hold[_i]) _hold[_i][0]=(char)_i; }}
    p[0] = 0xCD; free(p); printf("done\\n"); _exit(0);
}}
"""
    elif v == "thread":
        return f"""/* T3 DF thread ({n} holds) */
{_inc()}
#include <pthread.h>
static char *g=NULL; static volatile int r=0;
void *wrk(void *a){{ (void)a; while(!r); free(g); printf("df done\\n"); return NULL; }}
int main(void) {{
    g=(char*)malloc({sz}); if(!g) return 1; memset(g,0xAB,{sz});
    pthread_t th; pthread_create(&th,NULL,wrk,NULL);
    free(g);
{EXHAUST}
    char *_hold[{n}];
    for (int _i = 0; _i < {n}; _i++) {{ _hold[_i]=(char*)malloc({sz}); if(_hold[_i]) _hold[_i][0]=(char)_i; }}
    r=1; pthread_join(th,NULL); _exit(0);
}}
"""
    elif v == "cpp":
        return f"""/* T3 DF C++ ({n} holds) */
{_inc_cpp()}
class O{{ public: char d[56]; int id; O(int i):id(i){{}} virtual ~O(){{}} }};
int main(void) {{
    O *o=new O(42); printf("id=%d\\n",o->id); delete o;
{EXHAUST}
    char *_hold[{n}];
    for (int _i = 0; _i < {n}; _i++) {{ _hold[_i]=new char[{sz}]; if(_hold[_i]) _hold[_i][0]=(char)_i; }}
    delete o; _exit(0);
}}
"""


# ============================================================
# T4: Data structure traversal UAF (12 tests)
# ============================================================

def gen_t4():
    tests = []

    for i, L in enumerate([8, 64]):
        name = f"t4_ds_{i+1:02d}"
        write_test("cat1_temporal/generated", name, _t4_list(L))
        tests.append(name)
    for i, D in enumerate([4, 7]):
        name = f"t4_ds_{i+3:02d}"
        write_test("cat1_temporal/generated", name, _t4_tree(D))
        tests.append(name)
    name = "t4_ds_05"
    write_test("cat1_temporal/generated", name, _t4_hash(16))
    tests.append(name)

    name = "t4_ds_w01"
    write_test("cat1_temporal/generated", name, _t4_list(8, write=1))
    tests.append(name)
    name = "t4_ds_w02"
    write_test("cat1_temporal/generated", name, _t4_tree(4, write=1))
    tests.append(name)
    name = "t4_ds_w03"
    write_test("cat1_temporal/generated", name, _t4_hash(16, write=1))
    tests.append(name)

    name = "t4_ds_t01"
    write_test("cat1_temporal/generated", name, _t4_list(8, thread=1))
    tests.append(name)
    name = "t4_ds_t02"
    write_test("cat1_temporal/generated", name, _t4_tree(4, thread=1))
    tests.append(name)

    name = "t4_ds_c01"
    write_test("cat1_temporal/generated", name, _t4_cpp_vec(), ext=".cpp")
    tests.append(name)
    name = "t4_ds_c02"
    write_test("cat1_temporal/generated", name, _t4_cpp_map(), ext=".cpp")
    tests.append(name)

    return tests


def _t4_list(L, write=0, thread=0):
    half = L // 2
    wline = 'mid->v = 0xCD; printf("wrote\\n");' if write else 'printf("%%d ", cur->v);'
    th = ""
    if thread:
        th = '\n#include <pthread.h>\nvoid *trav(void *a) { typedef struct N { int v; struct N* n; char pad[16]; } N; N *cur=(N*)a; while(cur){printf("%%d ",cur->v);cur=cur->n;} printf("\\n"); return NULL; }'
    return f"""/* T4 DS list L={L} {'w' if write else 'r'}{' thread' if thread else ''} */
{_inc()}{th}
typedef struct N {{ int v; struct N* n; char pad[16]; }} N;
int main(void) {{
    N *head = NULL;
    for (int i = 0; i < {L}; i++) {{ N *nn = (N*)malloc(sizeof(N)); nn->v = i; nn->n = head; head = nn; }}
    N *mid = head;
    for (int i = 0; i < {half}; i++) mid = mid->n;
    free(mid);
{EXHAUST}
    N *d = (N*)malloc(sizeof(N));
    N *cur = head;
    while (cur) {{ {wline} cur = cur->n; }}
    printf("\\n"); _exit(0);
}}
"""


def _t4_tree(D, write=0, thread=0):
    wline = 'sub->r->v = 0xCD;' if write else 'printf("UAF=%d\\\\n", sub->r->v);'
    th = '\n#include <pthread.h>\nvoid *trav(void *a) { printf("th done\\\\n"); return NULL; }' if thread else ""
    return f"""/* T4 DS tree D={D} {'w' if write else 'r'}{' thread' if thread else ''} */
{_inc()}{th}
typedef struct T {{ int v; struct T *l, *r; char pad[8]; }} T;
T *mk(int v, int d) {{ if (d >= {D}) return NULL; T *n = (T*)malloc(sizeof(T)); n->v = v; n->l = mk(v*2,d+1); n->r = mk(v*2+1,d+1); return n; }}
int main(void) {{
    T *root = mk(1,1); T *sub = root ? root->l : NULL;
    if (sub) {{ free(sub->r); }}
{EXHAUST}
    T *d = (T*)malloc(sizeof(T));
    if (sub && sub->r) {{ {wline} }}
    printf("done\\n"); _exit(0);
}}
"""


def _t4_hash(SZ, write=0):
    wline = "s->v = 0xCD; printf(\"wrote stale=%d\\\\n\", s->v);" if write else 'printf("stale k=%d v=%d\\\\n", s->k, s->v);'
    return f"""/* T4 DS hash SZ={SZ} {'w' if write else 'r'} */
{_inc()}
#define HS {SZ}
typedef struct E {{ int k, v; struct E* n; char pad[12]; }} E;
static E *ht[HS];
void ins(int k, int v) {{ int i = k % HS; E *e = (E*)malloc(sizeof(E)); e->k = k; e->v = v; e->n = ht[i]; ht[i] = e; }}
E *lu(int k) {{ E *e = ht[k % HS]; while (e && e->k != k) e = e->n; return e; }}
int main(void) {{
    for (int i = 0; i < HS; i++) ht[i] = NULL;
    for (int i = 0; i < {SZ*2}; i++) ins(i, i*10);
    E *e = lu({SZ}); if (e) {{ printf("found k=%d v=%d\\n", e->k, e->v); free(e); }}
{EXHAUST}
    E *d = (E*)malloc(sizeof(E));
    E *s = lu({SZ}); if (s) {{ {wline} }}
    _exit(0);
}}
"""


def _t4_cpp_vec():
    return f"""/* T4 DS C++ vector iterator UAF */
{_inc_cpp()}
struct Vec {{ int *data; int cap, len; }};
void vp(Vec *v, int x) {{ if(v->len>=v->cap){{int nc=v->cap?v->cap*2:4;int*nd=(int*)malloc(nc*4);for(int i=0;i<v->len;i++)nd[i]=v->data[i];free(v->data);v->data=nd;v->cap=nc;}}v->data[v->len++]=x; }}
int main(void) {{
    Vec v={{NULL,0,0}}; for(int i=0;i<10;i++)vp(&v,i*10);
    int *old=v.data; for(int i=0;i<20;i++)vp(&v,i*100);
{EXHAUST}
    char *d=(char*)malloc(64);d[0]='D';
    printf("stale=%d\\n",old[0]);_exit(0);
}}
"""


def _t4_cpp_map():
    return f"""/* T4 DS C++ tree node UAF */
{_inc_cpp()}
struct N {{ int k,v; N *l,*r; }};
N *mk(int k,int v){{N*n=(N*)malloc(sizeof(N));n->k=k;n->v=v;n->l=n->r=NULL;return n;}}
int main(void) {{
    N *root=mk(50,500); root->l=mk(30,300);root->r=mk(70,700);root->l->l=mk(20,200);root->l->r=mk(40,400);
    free(root->l->r);
{EXHAUST}
    N *d=(N*)malloc(sizeof(N));
    N *st=root->l->r; if(st)printf("stale k=%d v=%d\\n",st->k,st->v);
    _exit(0);
}}
"""


# ============================================================
# T5: C++ object lifetime + realloc chain (12 tests)
# ============================================================

def gen_t5():
    tests = []
    srcs = [
        ("t5_cpp_01", _t5_vcall()), ("t5_cpp_02", _t5_member()),
        ("t5_cpp_03", _t5_arr_mismatch()), ("t5_cpp_04", _t5_downcast()),
        ("t5_cpp_05", _t5_realloc()),
    ]
    for name, src in srcs:
        write_test("cat1_temporal/generated", name, src, ext=".cpp")
        tests.append(name)
    for i, fn in enumerate([_t5_vcall_w, _t5_member_w, _t5_arr_mismatch_w]):
        name = f"t5_cpp_w{i+1:02d}"
        write_test("cat1_temporal/generated", name, fn(), ext=".cpp")
        tests.append(name)
    name = "t5_cpp_t01"
    write_test("cat1_temporal/generated", name, _t5_thread_vcall(), ext=".cpp")
    tests.append(name)
    name = "t5_cpp_t02"
    write_test("cat1_temporal/generated", name, _t5_thread_member(), ext=".cpp")
    tests.append(name)
    name = "t5_cpp_e01"
    write_test("cat1_temporal/generated", name, _t5_template(), ext=".cpp")
    tests.append(name)
    name = "t5_cpp_e02"
    write_test("cat1_temporal/generated", name, _t5_inheritance(), ext=".cpp")
    tests.append(name)
    return tests


def _t5_vcall():
    return f"""/* T5 C++ virtual call after delete */
{_inc_cpp()}
class Base {{ public: char d[56]; int id; Base(int i):id(i){{}} virtual int g(){{return id;}} virtual ~Base(){{}} }};
int main(void) {{ Base *o=new Base(42); printf("id=%d\\n",o->g()); delete o;
{EXHAUST}
char *x=new char[64];x[0]='D'; printf("stale=%d\\n",o->g()); _exit(0); }}
"""

def _t5_member():
    return f"""/* T5 C++ member access after delete */
{_inc_cpp()}
struct D {{ char b[56]; int v; double s; D():v(99),s(3.14){{}} void p(){{printf("v=%d s=%.2f\\n",v,s);}} }};
int main(void) {{ D *d=new D(); d->p(); delete d;
{EXHAUST}
char *x=new char[64];x[0]='D'; d->p(); _exit(0); }}
"""

def _t5_arr_mismatch():
    return f"""/* T5 C++ new[]/delete mismatch */
{_inc_cpp()}
struct E {{ char d[28]; int id; E():id(0){{}} }};
int main(void) {{ E *a=new E[5]; for(int i=0;i<5;i++)a[i].id=i*10; printf("a[2]=%d\\n",a[2].id); delete a;
{EXHAUST}
char *x=new char[64];x[0]='D'; printf("stale=%d\\n",a[2].id); _exit(0); }}
"""

def _t5_downcast():
    return f"""/* T5 C++ downcast after delete */
{_inc_cpp()}
class A{{public:char d[48];int age;A():age(5){{}}virtual~A(){{}}virtual const char*s(){{return"?";}}}};
class D:public A{{public:int bc;D():bc(0){{}}const char*s(){{bc++;return"woof";}}}};
int main(void){{A*a=new D();printf("s=%s\\n",a->s());delete a;
{EXHAUST}
char*x=new char[64];x[0]='D';printf("stale=%s\\n",a->s());_exit(0);}}
"""

def _t5_realloc():
    return f"""/* T5 C++ realloc chain UAF (new+copy+delete simulates realloc) */
{_inc_cpp()}
int main(void) {{
    int n=5; char *ptrs[5];
    ptrs[0]=new char[64]; memset(ptrs[0],0x11,64);
    for(int i=1;i<n;i++){{ ptrs[i]=new char[64+i*4096]; memcpy(ptrs[i],ptrs[i-1],64+(i-1)*4096); delete[] ptrs[i-1]; }}
{EXHAUST}
    char *d=new char[64]; d[0]='D';
    volatile char c=ptrs[0][0]; printf("ptrs[0][0]=%02x\\n",(unsigned char)c); _exit(0);
}}
"""

def _t5_vcall_w():
    return f"""/* T5 C++ virtual write after delete */
{_inc_cpp()}
class W{{public:char d[56];int id;W(int i):id(i){{}}virtual int g(){{return id;}}virtual void s(int i){{id=i;}}virtual~W(){{}}}};
int main(void){{W*w=new W(42);printf("id=%d\\n",w->g());delete w;
{EXHAUST}
char*x=new char[64];x[0]='D';w->s(99);printf("stale=%d\\n",w->g());_exit(0);}}
"""

def _t5_member_w():
    return f"""/* T5 C++ member write after delete */
{_inc_cpp()}
struct R{{char b[48];int c;double rt;R():c(0),rt(0.0){{}}}};
int main(void){{R*r=new R();printf("c=%d\\n",r->c);delete r;
{EXHAUST}
char*x=new char[64];x[0]='D';r->c=999;printf("stale=%d\\n",r->c);_exit(0);}}
"""

def _t5_arr_mismatch_w():
    return f"""/* T5 C++ array mismatch write after delete */
{_inc_cpp()}
struct C{{char d[28];int v;C():v(0){{}}}};
int main(void){{C*a=new C[5];for(int i=0;i<5;i++)a[i].v=i*10;printf("a[3]=%d\\n",a[3].v);delete a;
{EXHAUST}
char*x=new char[64];x[0]='D';a[3].v=777;printf("stale=%d\\n",a[3].v);_exit(0);}}
"""

def _t5_thread_vcall():
    return f"""/* T5 C++ thread virtual call after delete */
{_inc_cpp()}
#include <pthread.h>
class O{{public:char d[56];int id;O(int i):id(i){{}}virtual int g(){{return id;}}virtual~O(){{}}}};
static O*g=NULL;static volatile int r=0;
void*wrk(void*a){{(void)a;while(!r);printf("wrk=%d\\n",g->g());return NULL;}}
int main(void){{g=new O(42);printf("id=%d\\n",g->g());pthread_t th;pthread_create(&th,NULL,wrk,NULL);delete g;
{EXHAUST}
char*x=new char[64];x[0]='D';r=1;pthread_join(th,NULL);_exit(0);}}
"""

def _t5_thread_member():
    return f"""/* T5 C++ thread member after delete */
{_inc_cpp()}
#include <pthread.h>
struct D{{char b[56];int c;D():c(0){{}}void inc(){{c++;}}}};
static D*g=NULL;static volatile int r=0;
void*wrk(void*a){{(void)a;while(!r);g->inc();printf("wrk=%d\\n",g->c);return NULL;}}
int main(void){{g=new D();printf("c=%d\\n",g->c);pthread_t th;pthread_create(&th,NULL,wrk,NULL);delete g;
{EXHAUST}
char*x=new char[64];x[0]='D';r=1;pthread_join(th,NULL);_exit(0);}}
"""

def _t5_template():
    return f"""/* T5 C++ template UAF */
{_inc_cpp()}
template<typename T>class Box{{T val;char pad[48];public:Box(T v):val(v){{}}T g(){{return val;}}virtual~Box(){{}}}};
int main(void){{Box<int>*b=new Box<int>(42);printf("v=%d\\n",b->g());delete b;
{EXHAUST}
char*x=new char[64];x[0]='D';printf("stale=%d\\n",b->g());_exit(0);}}
"""

def _t5_inheritance():
    return f"""/* T5 C++ deep inheritance UAF */
{_inc_cpp()}
class L0{{public:char d0[48];int v0;L0():v0(10){{}}virtual int g(){{return v0;}}virtual~L0(){{}}}};
class L1:public L0{{public:int v1;L1():v1(20){{}}int g(){{return v1;}}}};
class L2:public L1{{public:int v2;L2():v2(30){{}}int g(){{return v2;}}}};
class L3:public L2{{public:int v3;L3():v3(40){{}}int g(){{return v3;}}}};
class L4:public L3{{public:int v4;L4():v4(50){{}}int g(){{return v4;}}}};
int main(void){{L0*o=new L4();printf("g=%d\\n",o->g());delete o;
{EXHAUST}
char*x=new char[64];x[0]='D';printf("stale=%d\\n",o->g());_exit(0);}}
"""


# ============================================================
# Spatial generators (unchanged)
# ============================================================

def gen_oob_skip():
    tests = []
    idx = 0
    for sz in [16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192]:
        for off in [1, sz // 2, sz - 1]:
            if off <= 0 or off > 1000000: continue
            idx += 1; name = f"oob_skip_{idx:03d}"
            vars_ = {
                "test_name": name, "src_size": sz, "target_size": sz,
                "oob_offset": off,
                "target_desc": f"adjacent {sz}B object",
                "expect_rsan": "MISS (spatial passes)",
                "expect_mixsan": "DETECT (63/64)",
                "extra_allocs": "", "extra_frees": ""
            }
            content = render(os.path.join(TEMPLATE_DIR, "oob_skip.c.tmpl"), vars_)
            write_test("cat2_spatial/generated", name, content)
            tests.append((name, vars_))
    return tests


def gen_ringbuf():
    tests = []
    for idx, buf_size in enumerate([4, 8, 16, 32, 64, 128, 256], 1):
        name = f"ringbuf_{idx:03d}"
        n_bytes = buf_size * 4
        n_overflow = buf_size // 2
        src = f"""/* Ring buffer overflow size={buf_size} */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#define MEMTAG_MASK 0x01FFFFFFFFFFFFFFULL
int main() {{
    int *ring = (int*)malloc({n_bytes});
    int *guard = (int*)malloc({n_bytes});
    if (!ring || !guard) return 1;
    memset(guard, 0xFF, {n_bytes});
    uintptr_t raw_ring = (uintptr_t)ring & MEMTAG_MASK;
    uintptr_t raw_guard = (uintptr_t)guard & MEMTAG_MASK;
    intptr_t dist = (intptr_t)(raw_guard - raw_ring);
    if (dist > 0 && dist < 100000) {{
        int *target = (int*)((char*)ring + dist);
        for (int i = 0; i < {n_overflow}; i++) target[i] = i * 10;
        printf("guard[0]=0x%x\\n", guard[0]);
    }} else {{ printf("skip\\n"); }}
    _exit(0);
}}
"""
        write_test("cat2_spatial/generated", name, src)
        tests.append((name, {"type": "ringbuf", "size": buf_size}))
    return tests


# ============================================================
# Main
# ============================================================

def main():
    gens = [
        ("cat1_temporal/generated", [gen_t1, gen_t3, gen_t4, gen_t5]),
        ("cat2_spatial/generated", [gen_oob_skip, gen_ringbuf]),
    ]

    grand_total = 0
    for cat, gen_funcs in gens:
        cat_total = 0
        for gen_func in gen_funcs:
            tests = gen_func()
            cat_total += len(tests)
        print(f"  {cat}: {cat_total} tests generated")
        grand_total += cat_total

    hw_count = 0
    for cat_dir in ["cat1_temporal", "cat2_spatial", "cat3_performance"]:
        d = os.path.join(SCRIPT_DIR, cat_dir)
        if os.path.isdir(d):
            for f in os.listdir(d):
                if (f.endswith(".c") or f.endswith(".cpp")) and "generated" not in os.path.join(cat_dir, f):
                    hw_count += 1
    print(f"\nTotal generated: {grand_total}")
    print(f"Existing hand-written: {hw_count}")
    print(f"Grand total: {grand_total + hw_count}")


if __name__ == "__main__":
    main()
