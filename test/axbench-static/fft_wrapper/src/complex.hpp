#ifndef __COMPLEX_HPP__
#define __COMPLEX_HPP__

#define ANNOTATION_RANGE_N "scalar(range('_MIN_ANNOTATION_RANGE_N' , '_MAX_ANNOTATION_RANGE_N') final disabled)"
#define ANNOTATION_COMPLEX(R1,R2) "struct[scalar(" R1 "),scalar(" R2 ")]"
#define ANNOTATION_COMPLEX_RANGE ANNOTATION_COMPLEX("range('_MIN_ANNOTATION_COMPLEX_RANGE' , '_MAX_ANNOTATION_COMPLEX_RANGE') final", "range('_MIN_ANNOTATION_COMPLEX_RANGE' , '_MAX_ANNOTATION_COMPLEX_RANGE') final")

#define PI 3.1415926535897931

#if 1
typedef struct {
   float real;
   float imag;
} Complex;
#endif

void fftSinCos(float x, float* s, float* c);

#if 0
float abs(const Complex* x);
float arg(const Complex* x);
#endif

#endif
