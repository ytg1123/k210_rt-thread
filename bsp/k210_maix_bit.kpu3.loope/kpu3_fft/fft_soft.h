#ifndef _FFT_SOFT_H
#define _FFT_SOFT_H

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

/* Software FFT Function  */
/* for K210 rt-theread */
/* code at 2022.07.27 by yoshito tagashira */

#define SWAP(a, b) do {complex t = (a); (a) = (b); (b) = t;} while(0)

typedef struct{double real, imag;} complex;

void fft_soft(complex *data, int n);
void ifft_soft(complex *data, int n);
void show(complex *data, int n);

#endif /* _FFT_SOFT_H */
