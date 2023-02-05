// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -emit-llvm -fforce-enable-int128 %s -o - \
// RUN:   | FileCheck %s -check-prefixes=CHECK,CHECK-FORCEINT128
// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -target-feature +f -target-abi ilp32f -emit-llvm %s -o - \
// RUN:     | FileCheck %s
// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -target-feature +d -target-feature +f -target-abi ilp32d -emit-llvm %s -o - \
// RUN:     | FileCheck %s
// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -target-abi ilp32e -emit-llvm %s -o - \
// RUN:     | FileCheck %s

// This file contains test cases that will have the same output for the ilp32,
// ilp32e, ilp32f, and ilp32d ABIs.

#include <stddef.h>
#include <stdint.h>

// CHECK-LABEL: define{{.*}} void @f_void()
void f_void(void) {}

// Scalar arguments and return values smaller than the word size are extended
// according to the sign of their type, up to 32 bits

// CHECK-LABEL: define{{.*}} zeroext i1 @f_scalar_0(i1 noundef zeroext %x)
_Bool f_scalar_0(_Bool x) { return x; }

// CHECK-LABEL: define{{.*}} signext i8 @f_scalar_1(i8 noundef signext %x)
int8_t f_scalar_1(int8_t x) { return x; }

// CHECK-LABEL: define{{.*}} zeroext i8 @f_scalar_2(i8 noundef zeroext %x)
uint8_t f_scalar_2(uint8_t x) { return x; }

// CHECK-LABEL: define{{.*}} i32 @f_scalar_3(i32 noundef %x)
int32_t f_scalar_3(int32_t x) { return x; }

// CHECK-LABEL: define{{.*}} i64 @f_scalar_4(i64 noundef %x)
int64_t f_scalar_4(int64_t x) { return x; }

#ifdef __SIZEOF_INT128__
// CHECK-FORCEINT128-LABEL: define{{.*}} i128 @f_scalar_5(i128 noundef %x)
__int128_t f_scalar_5(__int128_t x) { return x; }
#endif

// CHECK-LABEL: define{{.*}} float @f_fp_scalar_1(float noundef %x)
float f_fp_scalar_1(float x) { return x; }

// CHECK-LABEL: define{{.*}} double @f_fp_scalar_2(double noundef %x)
double f_fp_scalar_2(double x) { return x; }

// Scalars larger than 2*xlen are passed/returned indirect. However, the
// RISC-V LLVM backend can handle this fine, so the function doesn't need to
// be modified.

// CHECK-LABEL: define{{.*}} fp128 @f_fp_scalar_3(fp128 noundef %x)
long double f_fp_scalar_3(long double x) { return x; }

// Empty structs or unions are ignored.

struct empty_s {};

// CHECK-LABEL: define{{.*}} void @f_agg_empty_struct()
struct empty_s f_agg_empty_struct(struct empty_s x) {
  return x;
}

union empty_u {};

// CHECK-LABEL: define{{.*}} void @f_agg_empty_union()
union empty_u f_agg_empty_union(union empty_u x) {
  return x;
}

// Aggregates <= 2*xlen may be passed in registers, so will be coerced to
// integer arguments. The rules for return are the same.

struct tiny {
  uint8_t a, b, c, d;
};

// CHECK-LABEL: define{{.*}} void @f_agg_tiny(i32 %x.coerce)
void f_agg_tiny(struct tiny x) {
  x.a += x.b;
  x.c += x.d;
}

// CHECK-LABEL: define{{.*}} i32 @f_agg_tiny_ret()
struct tiny f_agg_tiny_ret() {
  return (struct tiny){1, 2, 3, 4};
}

typedef uint8_t v4i8 __attribute__((vector_size(4)));
typedef int32_t v1i32 __attribute__((vector_size(4)));

// CHECK-LABEL: define{{.*}} void @f_vec_tiny_v4i8(i32 noundef %x.coerce)
void f_vec_tiny_v4i8(v4i8 x) {
  x[0] = x[1];
  x[2] = x[3];
}

// CHECK-LABEL: define{{.*}} i32 @f_vec_tiny_v4i8_ret()
v4i8 f_vec_tiny_v4i8_ret() {
  return (v4i8){1, 2, 3, 4};
}

// CHECK-LABEL: define{{.*}} void @f_vec_tiny_v1i32(i32 noundef %x.coerce)
void f_vec_tiny_v1i32(v1i32 x) {
  x[0] = 114;
}

// CHECK-LABEL: define{{.*}} i32 @f_vec_tiny_v1i32_ret()
v1i32 f_vec_tiny_v1i32_ret() {
  return (v1i32){1};
}

struct small {
  int32_t a, *b;
};

// CHECK-LABEL: define{{.*}} void @f_agg_small([2 x i32] %x.coerce)
void f_agg_small(struct small x) {
  x.a += *x.b;
  x.b = &x.a;
}

// CHECK-LABEL: define{{.*}} [2 x i32] @f_agg_small_ret()
struct small f_agg_small_ret() {
  return (struct small){1, 0};
}

typedef uint8_t v8i8 __attribute__((vector_size(8)));
typedef int64_t v1i64 __attribute__((vector_size(8)));

// CHECK-LABEL: define{{.*}} void @f_vec_small_v8i8(i64 noundef %x.coerce)
void f_vec_small_v8i8(v8i8 x) {
  x[0] = x[7];
}

// CHECK-LABEL: define{{.*}} i64 @f_vec_small_v8i8_ret()
v8i8 f_vec_small_v8i8_ret() {
  return (v8i8){1, 2, 3, 4, 5, 6, 7, 8};
}

// CHECK-LABEL: define{{.*}} void @f_vec_small_v1i64(i64 noundef %x.coerce)
void f_vec_small_v1i64(v1i64 x) {
  x[0] = 114;
}

// CHECK-LABEL: define{{.*}} i64 @f_vec_small_v1i64_ret()
v1i64 f_vec_small_v1i64_ret() {
  return (v1i64){1};
}

// Aggregates of 2*xlen size and 2*xlen alignment should be coerced to a
// single 2*xlen-sized argument, to ensure that alignment can be maintained if
// passed on the stack.

struct small_aligned {
  int64_t a;
};

// CHECK-LABEL: define{{.*}} void @f_agg_small_aligned(i64 %x.coerce)
void f_agg_small_aligned(struct small_aligned x) {
  x.a += x.a;
}

// CHECK-LABEL: define{{.*}} i64 @f_agg_small_aligned_ret(i64 %x.coerce)
struct small_aligned f_agg_small_aligned_ret(struct small_aligned x) {
  return (struct small_aligned){10};
}

// Aggregates greater > 2*xlen will be passed and returned indirectly
struct large {
  int32_t a, b, c, d;
};

// CHECK-LABEL: define{{.*}} void @f_agg_large(%struct.large* noundef %x)
void f_agg_large(struct large x) {
  x.a = x.b + x.c + x.d;
}

// The address where the struct should be written to will be the first
// argument
// CHECK-LABEL: define{{.*}} void @f_agg_large_ret(%struct.large* noalias sret(%struct.large) align 4 %agg.result, i32 noundef %i, i8 noundef signext %j)
struct large f_agg_large_ret(int32_t i, int8_t j) {
  return (struct large){1, 2, 3, 4};
}

typedef unsigned char v16i8 __attribute__((vector_size(16)));

// CHECK-LABEL: define{{.*}} void @f_vec_large_v16i8(<16 x i8>* noundef %0)
void f_vec_large_v16i8(v16i8 x) {
  x[0] = x[7];
}

// CHECK-LABEL: define{{.*}} void @f_vec_large_v16i8_ret(<16 x i8>* noalias sret(<16 x i8>) align 16 %agg.result)
v16i8 f_vec_large_v16i8_ret() {
  return (v16i8){1, 2, 3, 4, 5, 6, 7, 8};
}

// Aggregates and >=XLen scalars passed on the stack should be lowered just as
// they would be if passed via registers.

// CHECK-LABEL: define{{.*}} void @f_scalar_stack_5(double noundef %a, i64 noundef %b, double noundef %c, i64 noundef %d, i32 noundef %e, i64 noundef %f, float noundef %g, double noundef %h, fp128 noundef %i)
void f_scalar_stack_5(double a, int64_t b, double c, int64_t d, int e,
                      int64_t f, float g, double h, long double i) {}

// CHECK-LABEL: define{{.*}} void @f_agg_stack(double noundef %a, i64 noundef %b, double noundef %c, i64 noundef %d, i32 %e.coerce, [2 x i32] %f.coerce, i64 %g.coerce, %struct.large* noundef %h)
void f_agg_stack(double a, int64_t b, double c, int64_t d, struct tiny e,
                 struct small f, struct small_aligned g, struct large h) {}

// Ensure that ABI lowering happens as expected for vararg calls. For RV32
// with the base integer calling convention there will be no observable
// differences in the lowered IR for a call with varargs vs without.

int f_va_callee(int, ...);

// CHECK-LABEL: define{{.*}} void @f_va_caller()
// CHECK: call i32 (i32, ...) @f_va_callee(i32 noundef 1, i32 noundef 2, i64 noundef 3, double noundef 4.000000e+00, double noundef 5.000000e+00, i32 {{%.*}}, [2 x i32] {{%.*}}, i64 {{%.*}}, %struct.large* noundef {{%.*}})
void f_va_caller() {
  f_va_callee(1, 2, 3LL, 4.0f, 5.0, (struct tiny){6, 7, 8, 9},
              (struct small){10, NULL}, (struct small_aligned){11},
              (struct large){12, 13, 14, 15});
}

// CHECK-LABEL: define{{.*}} i32 @f_va_1(i8* noundef %fmt, ...) {{.*}} {
// CHECK:   [[FMT_ADDR:%.*]] = alloca i8*, align 4
// CHECK:   [[VA:%.*]] = alloca i8*, align 4
// CHECK:   [[V:%.*]] = alloca i32, align 4
// CHECK:   store i8* %fmt, i8** [[FMT_ADDR]], align 4
// CHECK:   [[VA1:%.*]] = bitcast i8** [[VA]] to i8*
// CHECK:   call void @llvm.va_start(i8* [[VA1]])
// CHECK:   [[ARGP_CUR:%.*]] = load i8*, i8** [[VA]], align 4
// CHECK:   [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR]], i32 4
// CHECK:   store i8* [[ARGP_NEXT]], i8** [[VA]], align 4
// CHECK:   [[TMP0:%.*]] = bitcast i8* [[ARGP_CUR]] to i32*
// CHECK:   [[TMP1:%.*]] = load i32, i32* [[TMP0]], align 4
// CHECK:   store i32 [[TMP1]], i32* [[V]], align 4
// CHECK:   [[VA2:%.*]] = bitcast i8** [[VA]] to i8*
// CHECK:   call void @llvm.va_end(i8* [[VA2]])
// CHECK:   [[TMP2:%.*]] = load i32, i32* [[V]], align 4
// CHECK:   ret i32 [[TMP2]]
// CHECK: }
int f_va_1(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  int v = __builtin_va_arg(va, int);
  __builtin_va_end(va);

  return v;
}
