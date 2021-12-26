#ifndef _SCALER_COEFF_H_
#define _SCALER_COEFF_H_

#define ABS(x)                ( (x) >= 0 ? (x) : (-x))

int mpa_mul(unsigned int x1, unsigned int x2);
int mpa_mulu(unsigned int x1, unsigned int x2, unsigned int *hi, unsigned int *lo);
int mpa_mul_q30(int x1, int x2);
int mpa_mul_q29(int x1, int x2);
int mpa_mul_q28(int x1, int x2);
unsigned int mpa_div64(unsigned int n_hi, unsigned int n_lo, unsigned int d);
int Cubic(int s, unsigned int m);
int Linear (int s, unsigned int m);
#endif

