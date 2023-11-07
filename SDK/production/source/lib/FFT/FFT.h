/**
 * @file FFT.h
 * @date 11 Novmebr 2021
 * @brief This module implements FFt related algorithms
 *
 *
 *
 */
/********************************************************************
*
* Module Name: FFT.h
* Design:
* Implement FFt related algorithms
*
********************************************************************/
#ifndef FFT_H
#define FFT_H
/********************************************************************
* IMPORTS
********************************************************************/
/********************************************************************
* EXPORTED CONSTANTS
********************************************************************/
#define PI 3.1416
/********************************************************************
* EXPORTED TYPES
********************************************************************/
typedef struct _complex_t
{
	double	real;
	double	img;
} complex_t;

extern double hanning[2048];
/********************************************************************
* EXPORTED MACROS
********************************************************************/
/********************************************************************
* EXPORTED FUNCTIONS
********************************************************************/
void complex_sqr_abs(complex_t xi_z, double *xo_abs);
void fft(complex_t *x,int m);
/********************************************************************
* INTERNAL FUNCTIONS DECLARATIONS (FOR UNIT TESTING)
********************************************************************/

#endif // FFT_H
