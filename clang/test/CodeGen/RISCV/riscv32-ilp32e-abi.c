// RUN: %clang_cc1 -no-opaque-pointers -triple riscv32 -emit-llvm -target-abi ilp32e %s -o - \
// RUN:     | FileCheck -check-prefix=ILP32E %s
// RUN: not %clang_cc1 -no-opaque-pointers -triple riscv32 -target-feature +d -target-feature +f -emit-llvm -target-abi ilp32e %s 2>&1 \
// RUN:     | FileCheck -check-prefix=ILP32E-WITH-FD %s

// This file contains test cases for only ilp32e.

// ILP32E-WITH-FD: error: invalid feature combination: ILP32E must not be used with the D ISA extension

#include <stddef.h>
#include <stdint.h>

struct tiny {
  uint8_t a, b, c, d;
};

struct small {
  int32_t a, *b;
};

struct small_aligned {
  int64_t a;
};

struct large {
  int32_t a, b, c, d;
};

// Scalars passed on the stack should not have signext/zeroext attributes
// (they are anyext).

// ILP32E-LABEL: define{{.*}} i32 @f_scalar_stack_1(i32 %a.coerce, [2 x i32] %b.coerce, %struct.large* noundef %c, i8 noundef zeroext %d, i8 noundef signext %e, i8 noundef %f, i8 noundef %g)
int f_scalar_stack_1(struct tiny a, struct small b, struct large c,
                     uint8_t d, int8_t e, uint8_t f, int8_t g) {
  return f + g;
}

// Ensure that scalars passed on the stack are still determined correctly in
// the presence of large return values that consume a register due to the need
// to pass a pointer.

// ILP32E-LABEL: define{{.*}} void @f_scalar_stack_2(%struct.large* noalias sret(%struct.large) align 4 %agg.result, i32 noundef %a, i64 noundef %b, fp128 noundef %c, i8 noundef zeroext %d, i8 noundef %e, i8 noundef %f)
struct large f_scalar_stack_2(int32_t a, int64_t b, long double c,
                              uint8_t d, int8_t e, uint8_t f) {
  return (struct large){a, d, e, f};
}

// ILP32E-LABEL: define{{.*}} fp128 @f_scalar_stack_4(i32 noundef %a, i64 noundef %b, fp128 noundef %c, i8 noundef zeroext %d, i8 noundef %e, i8 noundef %f)
long double f_scalar_stack_4(int32_t a, int64_t b, long double c,
                             uint8_t d, int8_t e, uint8_t f) {
  return c;
}

// Aggregates and >=XLen scalars passed on the stack should be lowered just as
// they would be if passed via registers.

// ILP32E-LABEL: define{{.*}} void @f_scalar_stack_5(double noundef %a, i64 noundef %b, double noundef %c, i32 noundef %d, i64 noundef %e, float noundef %f, double noundef %g, fp128 noundef %h)
void f_scalar_stack_5(double a, int64_t b, double c, int d,
                      int64_t e, float f, double g, long double h) {}

// ILP32E-LABEL: define{{.*}} void @f_agg_stack(double noundef %a, i64 noundef %b, double noundef %c, i32 %d.coerce, [2 x i32] %e.coerce, i64 %f.coerce, %struct.large* noundef %g)
void f_agg_stack(double a, int64_t b, double c, struct tiny d,
                 struct small e, struct small_aligned f, struct large g) {}

// An "aligned" register pair (where the first register is even-numbered) is
// used to pass varargs with 2x xlen alignment and 2x xlen size. While using
// ilp32e, we don't do such alignment.

// ILP32E-LABEL: @f_va_2(
// ILP32E-NEXT:  entry:
// ILP32E-NEXT:    [[FMT_ADDR:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[VA:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[V:%.*]] = alloca double, align 8
// ILP32E-NEXT:    store i8* [[FMT:%.*]], i8** [[FMT_ADDR]], align 4
// ILP32E-NEXT:    [[VA1:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_start(i8* [[VA1]])
// ILP32E-NEXT:    [[ARGP_CUR:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR]], i32 8
// ILP32E-NEXT:    store i8* [[ARGP_NEXT]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP0:%.*]] = bitcast i8* [[ARGP_CUR]] to double*
// ILP32E-NEXT:    [[TMP1:%.*]] = load double, double* [[TMP0]], align 4
// ILP32E-NEXT:    store double [[TMP1]], double* [[V]], align 8
// ILP32E-NEXT:    [[VA2:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_end(i8* [[VA2]])
// ILP32E-NEXT:    [[TMP2:%.*]] = load double, double* [[V]], align 8
// ILP32E-NEXT:    ret double [[TMP2]]
//
double f_va_2(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  double v = __builtin_va_arg(va, double);
  __builtin_va_end(va);

  return v;
}

// ILP32E-LABEL: @f_va_3(
// ILP32E-NEXT:  entry:
// ILP32E-NEXT:    [[FMT_ADDR:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[VA:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[V:%.*]] = alloca double, align 8
// ILP32E-NEXT:    [[W:%.*]] = alloca i32, align 4
// ILP32E-NEXT:    [[X:%.*]] = alloca double, align 8
// ILP32E-NEXT:    store i8* [[FMT:%.*]], i8** [[FMT_ADDR]], align 4
// ILP32E-NEXT:    [[VA1:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_start(i8* [[VA1]])
// ILP32E-NEXT:    [[ARGP_CUR:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR]], i32 8
// ILP32E-NEXT:    store i8* [[ARGP_NEXT]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP0:%.*]] = bitcast i8* [[ARGP_CUR]] to double*
// ILP32E-NEXT:    [[TMP1:%.*]] = load double, double* [[TMP0]], align 4
// ILP32E-NEXT:    store double [[TMP1]], double* [[V]], align 8
// ILP32E-NEXT:    [[ARGP_CUR2:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT3:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR2]], i32 4
// ILP32E-NEXT:    store i8* [[ARGP_NEXT3]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP2:%.*]] = bitcast i8* [[ARGP_CUR2]] to i32*
// ILP32E-NEXT:    [[TMP3:%.*]] = load i32, i32* [[TMP2]], align 4
// ILP32E-NEXT:    store i32 [[TMP3]], i32* [[W]], align 4
// ILP32E-NEXT:    [[ARGP_CUR4:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT5:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR4]], i32 8
// ILP32E-NEXT:    store i8* [[ARGP_NEXT5]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP4:%.*]] = bitcast i8* [[ARGP_CUR4]] to double*
// ILP32E-NEXT:    [[TMP5:%.*]] = load double, double* [[TMP4]], align 4
// ILP32E-NEXT:    store double [[TMP5]], double* [[X]], align 8
// ILP32E-NEXT:    [[VA6:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_end(i8* [[VA6]])
// ILP32E-NEXT:    [[TMP6:%.*]] = load double, double* [[V]], align 8
// ILP32E-NEXT:    [[TMP7:%.*]] = load double, double* [[X]], align 8
// ILP32E-NEXT:    [[ADD:%.*]] = fadd double [[TMP6]], [[TMP7]]
// ILP32E-NEXT:    ret double [[ADD]]
//
double f_va_3(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  double v = __builtin_va_arg(va, double);
  int w = __builtin_va_arg(va, int);
  double x = __builtin_va_arg(va, double);
  __builtin_va_end(va);

  return v + x;
}

// ILP32E-LABEL: @f_va_4(
// ILP32E-NEXT:  entry:
// ILP32E-NEXT:    [[FMT_ADDR:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[VA:%.*]] = alloca i8*, align 4
// ILP32E-NEXT:    [[V:%.*]] = alloca i32, align 4
// ILP32E-NEXT:    [[LD:%.*]] = alloca fp128, align 16
// ILP32E-NEXT:    [[TS:%.*]] = alloca [[STRUCT_TINY:%.*]], align 1
// ILP32E-NEXT:    [[SS:%.*]] = alloca [[STRUCT_SMALL:%.*]], align 4
// ILP32E-NEXT:    [[LS:%.*]] = alloca [[STRUCT_LARGE:%.*]], align 4
// ILP32E-NEXT:    [[RET:%.*]] = alloca i32, align 4
// ILP32E-NEXT:    store i8* [[FMT:%.*]], i8** [[FMT_ADDR]], align 4
// ILP32E-NEXT:    [[VA1:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_start(i8* [[VA1]])
// ILP32E-NEXT:    [[ARGP_CUR:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR]], i32 4
// ILP32E-NEXT:    store i8* [[ARGP_NEXT]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP0:%.*]] = bitcast i8* [[ARGP_CUR]] to i32*
// ILP32E-NEXT:    [[TMP1:%.*]] = load i32, i32* [[TMP0]], align 4
// ILP32E-NEXT:    store i32 [[TMP1]], i32* [[V]], align 4
// ILP32E-NEXT:    [[ARGP_CUR2:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT3:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR2]], i32 4
// ILP32E-NEXT:    store i8* [[ARGP_NEXT3]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP2:%.*]] = bitcast i8* [[ARGP_CUR2]] to fp128**
// ILP32E-NEXT:    [[TMP3:%.*]] = load fp128*, fp128** [[TMP2]], align 4
// ILP32E-NEXT:    [[TMP4:%.*]] = load fp128, fp128* [[TMP3]], align 4
// ILP32E-NEXT:    store fp128 [[TMP4]], fp128* [[LD]], align 16
// ILP32E-NEXT:    [[ARGP_CUR4:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT5:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR4]], i32 4
// ILP32E-NEXT:    store i8* [[ARGP_NEXT5]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP5:%.*]] = bitcast i8* [[ARGP_CUR4]] to %struct.tiny*
// ILP32E-NEXT:    [[TMP6:%.*]] = bitcast %struct.tiny* [[TS]] to i8*
// ILP32E-NEXT:    [[TMP7:%.*]] = bitcast %struct.tiny* [[TMP5]] to i8*
// ILP32E-NEXT:    call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 1 [[TMP6]], i8* align 4 [[TMP7]], i32 4, i1 false)
// ILP32E-NEXT:    [[ARGP_CUR6:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT7:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR6]], i32 8
// ILP32E-NEXT:    store i8* [[ARGP_NEXT7]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP8:%.*]] = bitcast i8* [[ARGP_CUR6]] to %struct.small*
// ILP32E-NEXT:    [[TMP9:%.*]] = bitcast %struct.small* [[SS]] to i8*
// ILP32E-NEXT:    [[TMP10:%.*]] = bitcast %struct.small* [[TMP8]] to i8*
// ILP32E-NEXT:    call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 [[TMP9]], i8* align 4 [[TMP10]], i32 8, i1 false)
// ILP32E-NEXT:    [[ARGP_CUR8:%.*]] = load i8*, i8** [[VA]], align 4
// ILP32E-NEXT:    [[ARGP_NEXT9:%.*]] = getelementptr inbounds i8, i8* [[ARGP_CUR8]], i32 4
// ILP32E-NEXT:    store i8* [[ARGP_NEXT9]], i8** [[VA]], align 4
// ILP32E-NEXT:    [[TMP11:%.*]] = bitcast i8* [[ARGP_CUR8]] to %struct.large**
// ILP32E-NEXT:    [[TMP12:%.*]] = load %struct.large*, %struct.large** [[TMP11]], align 4
// ILP32E-NEXT:    [[TMP13:%.*]] = bitcast %struct.large* [[LS]] to i8*
// ILP32E-NEXT:    [[TMP14:%.*]] = bitcast %struct.large* [[TMP12]] to i8*
// ILP32E-NEXT:    call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 [[TMP13]], i8* align 4 [[TMP14]], i32 16, i1 false)
// ILP32E-NEXT:    [[VA10:%.*]] = bitcast i8** [[VA]] to i8*
// ILP32E-NEXT:    call void @llvm.va_end(i8* [[VA10]])
// ILP32E-NEXT:    [[TMP15:%.*]] = load i32, i32* [[V]], align 4
// ILP32E-NEXT:    [[CONV:%.*]] = sitofp i32 [[TMP15]] to fp128
// ILP32E-NEXT:    [[TMP16:%.*]] = load fp128, fp128* [[LD]], align 16
// ILP32E-NEXT:    [[ADD:%.*]] = fadd fp128 [[CONV]], [[TMP16]]
// ILP32E-NEXT:    [[CONV11:%.*]] = fptosi fp128 [[ADD]] to i32
// ILP32E-NEXT:    store i32 [[CONV11]], i32* [[RET]], align 4
// ILP32E-NEXT:    [[TMP17:%.*]] = load i32, i32* [[RET]], align 4
// ILP32E-NEXT:    [[A:%.*]] = getelementptr inbounds [[STRUCT_TINY]], %struct.tiny* [[TS]], i32 0, i32 0
// ILP32E-NEXT:    [[TMP18:%.*]] = load i8, i8* [[A]], align 1
// ILP32E-NEXT:    [[CONV12:%.*]] = zext i8 [[TMP18]] to i32
// ILP32E-NEXT:    [[ADD13:%.*]] = add nsw i32 [[TMP17]], [[CONV12]]
// ILP32E-NEXT:    [[B:%.*]] = getelementptr inbounds [[STRUCT_TINY]], %struct.tiny* [[TS]], i32 0, i32 1
// ILP32E-NEXT:    [[TMP19:%.*]] = load i8, i8* [[B]], align 1
// ILP32E-NEXT:    [[CONV14:%.*]] = zext i8 [[TMP19]] to i32
// ILP32E-NEXT:    [[ADD15:%.*]] = add nsw i32 [[ADD13]], [[CONV14]]
// ILP32E-NEXT:    [[C:%.*]] = getelementptr inbounds [[STRUCT_TINY]], %struct.tiny* [[TS]], i32 0, i32 2
// ILP32E-NEXT:    [[TMP20:%.*]] = load i8, i8* [[C]], align 1
// ILP32E-NEXT:    [[CONV16:%.*]] = zext i8 [[TMP20]] to i32
// ILP32E-NEXT:    [[ADD17:%.*]] = add nsw i32 [[ADD15]], [[CONV16]]
// ILP32E-NEXT:    [[D:%.*]] = getelementptr inbounds [[STRUCT_TINY]], %struct.tiny* [[TS]], i32 0, i32 3
// ILP32E-NEXT:    [[TMP21:%.*]] = load i8, i8* [[D]], align 1
// ILP32E-NEXT:    [[CONV18:%.*]] = zext i8 [[TMP21]] to i32
// ILP32E-NEXT:    [[ADD19:%.*]] = add nsw i32 [[ADD17]], [[CONV18]]
// ILP32E-NEXT:    store i32 [[ADD19]], i32* [[RET]], align 4
// ILP32E-NEXT:    [[TMP22:%.*]] = load i32, i32* [[RET]], align 4
// ILP32E-NEXT:    [[A20:%.*]] = getelementptr inbounds [[STRUCT_SMALL]], %struct.small* [[SS]], i32 0, i32 0
// ILP32E-NEXT:    [[TMP23:%.*]] = load i32, i32* [[A20]], align 4
// ILP32E-NEXT:    [[ADD21:%.*]] = add nsw i32 [[TMP22]], [[TMP23]]
// ILP32E-NEXT:    [[B22:%.*]] = getelementptr inbounds [[STRUCT_SMALL]], %struct.small* [[SS]], i32 0, i32 1
// ILP32E-NEXT:    [[TMP24:%.*]] = load i32*, i32** [[B22]], align 4
// ILP32E-NEXT:    [[TMP25:%.*]] = ptrtoint i32* [[TMP24]] to i32
// ILP32E-NEXT:    [[ADD23:%.*]] = add nsw i32 [[ADD21]], [[TMP25]]
// ILP32E-NEXT:    store i32 [[ADD23]], i32* [[RET]], align 4
// ILP32E-NEXT:    [[TMP26:%.*]] = load i32, i32* [[RET]], align 4
// ILP32E-NEXT:    [[A24:%.*]] = getelementptr inbounds [[STRUCT_LARGE]], %struct.large* [[LS]], i32 0, i32 0
// ILP32E-NEXT:    [[TMP27:%.*]] = load i32, i32* [[A24]], align 4
// ILP32E-NEXT:    [[ADD25:%.*]] = add nsw i32 [[TMP26]], [[TMP27]]
// ILP32E-NEXT:    [[B26:%.*]] = getelementptr inbounds [[STRUCT_LARGE]], %struct.large* [[LS]], i32 0, i32 1
// ILP32E-NEXT:    [[TMP28:%.*]] = load i32, i32* [[B26]], align 4
// ILP32E-NEXT:    [[ADD27:%.*]] = add nsw i32 [[ADD25]], [[TMP28]]
// ILP32E-NEXT:    [[C28:%.*]] = getelementptr inbounds [[STRUCT_LARGE]], %struct.large* [[LS]], i32 0, i32 2
// ILP32E-NEXT:    [[TMP29:%.*]] = load i32, i32* [[C28]], align 4
// ILP32E-NEXT:    [[ADD29:%.*]] = add nsw i32 [[ADD27]], [[TMP29]]
// ILP32E-NEXT:    [[D30:%.*]] = getelementptr inbounds [[STRUCT_LARGE]], %struct.large* [[LS]], i32 0, i32 3
// ILP32E-NEXT:    [[TMP30:%.*]] = load i32, i32* [[D30]], align 4
// ILP32E-NEXT:    [[ADD31:%.*]] = add nsw i32 [[ADD29]], [[TMP30]]
// ILP32E-NEXT:    store i32 [[ADD31]], i32* [[RET]], align 4
// ILP32E-NEXT:    [[TMP31:%.*]] = load i32, i32* [[RET]], align 4
// ILP32E-NEXT:    ret i32 [[TMP31]]
//
int f_va_4(char *fmt, ...) {
  __builtin_va_list va;

  __builtin_va_start(va, fmt);
  int v = __builtin_va_arg(va, int);
  long double ld = __builtin_va_arg(va, long double);
  struct tiny ts = __builtin_va_arg(va, struct tiny);
  struct small ss = __builtin_va_arg(va, struct small);
  struct large ls = __builtin_va_arg(va, struct large);
  __builtin_va_end(va);

  int ret = (int)((long double)v + ld);
  ret = ret + ts.a + ts.b + ts.c + ts.d;
  ret = ret + ss.a + (int)ss.b;
  ret = ret + ls.a + ls.b + ls.c + ls.d;

  return ret;
}
