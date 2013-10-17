/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Hello Rotate",              /* Team name */

    "Chi Yeung Jonathan Ng",     /* First member full name */
    "jonathancy.ng@utoronto.ca",  /* First member email address */

    "Michael Law",                   /* Second member full name (leave blank if none) */
    "m.law@mail.utoronto.ca"                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst) 
{
    int i, j;

    for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

/*
 * ECE 454 Students: Write your rotate functions here:
 */ 

// a "transpose" helper function
void transpose(int dim, pixel *orig, pixel *rotated) 
{
    int i, j;
    for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			rotated[RIDX(j, i, dim)] = orig[RIDX(i, j, dim)];
}

// an "exchange" helper function
void exchange(int dim, pixel *orig, pixel *exchanged) 
{
    int i, j;
    for (i = 0; i < dim; i++)
		for (j = 0; j < dim; j++)
			exchanged[RIDX(dim-1-i, j, dim)] = orig[RIDX(i, j, dim)];
}

// second attempt
char rotate_two_descr[] = "Swapping the inner and outer loop to write in row-major";
void attempt_two(int dim, pixel *src, pixel *dst) 
{
    int i, j;

	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
}

// third attempt
char rotate_three_descr[] = "Unrolling";
void attempt_three(int dim, pixel *src, pixel *dst) 
{
    int i, j;

	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
		{
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
			i++;
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];
		}
}

// fourth attempt
char rotate_four_descr[] = "Try tiling";
void attempt_four(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int x, y;
	int T = 16;

	for (j = 0; j < dim; j+=T)
		for (i = 0; i < dim; i+=T)
			for (x = i; x < i+T; x++)
				for (y = j; y < j+T; y++)
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
}

// fifth attempt
char rotate_five_descr[] = "Try tiling with 32";
void attempt_five(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int x, y;
	int T = 32;

	for (j = 0; j < dim; j+=T)
		for (i = 0; i < dim; i+=T)
			for (x = i; x < i+T; x++)
				for (y = j; y < j+T; y++)
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
}

// sixth attempt
char rotate_six_descr[] = "Try tiling with 32 and swapping write into row-major";
void attempt_six(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int x, y;
	int T = 32;

	for (j = 0; j < dim; j+=T)
		for (i = 0; i < dim; i+=T)
			for (y = j; y < j+T; y++)
				for (x = i; x < i+T; x++)
				{
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
				}
}

// seventh attempt
char rotate_seven_descr[] = "Try tiling with 32 and swapping write into row-major, with unrolling of 4";
void attempt_seven(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int x, y;
	int T = 32;

	for (j = 0; j < dim; j+=T)
		for (i = 0; i < dim; i+=T)
			for (y = j; y < j+T; y++)
				for (x = i; x < i+T; x++)
				{
					dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
					dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
					dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
					dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
					x+=3;
				}
}

// eigth attempt
char rotate_eight_descr[] = "T = 32, write row-major, 4/8 loop unroll based on dim size";
void attempt_eight(int dim, pixel *src, pixel *dst) 
{
    int i, j;
    int x, y;
	int T = 32;

	if (dim <= 512)
		for (j = 0; j < dim; j+=T)
			for (i = 0; i < dim; i+=T)
				for (y = j; y < j+T; y++)
					for (x = i; x < i+T; x++)
					{
						dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
						dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
						dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
						dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
						x+=3;
					}
	else
		for (j = 0; j < dim; j+=T)
			for (i = 0; i < dim; i+=T)
				for (y = j; y < j+T; y++)
					for (x = i; x < i+T; x++)
					{
						dst[RIDX(dim-1-y, x, dim)] = src[RIDX(x, y, dim)];
						dst[RIDX(dim-1-y, x+1, dim)] = src[RIDX(x+1, y, dim)];
						dst[RIDX(dim-1-y, x+2, dim)] = src[RIDX(x+2, y, dim)];
						dst[RIDX(dim-1-y, x+3, dim)] = src[RIDX(x+3, y, dim)];
						dst[RIDX(dim-1-y, x+4, dim)] = src[RIDX(x+4, y, dim)];
						dst[RIDX(dim-1-y, x+5, dim)] = src[RIDX(x+5, y, dim)];
						dst[RIDX(dim-1-y, x+6, dim)] = src[RIDX(x+6, y, dim)];
						dst[RIDX(dim-1-y, x+7, dim)] = src[RIDX(x+7, y, dim)];
						x+=7;
					}
}
/* 
 * rotate - Your current working version of rotate
 * IMPORTANT: This is the version you will be graded on
 */
char rotate_descr[] = "rotate: Current working version";
void rotate(int dim, pixel *src, pixel *dst) 
{
    attempt_eight(dim, src, dst);
}





/*********************************************************************
 * register_rotate_functions - Register all of your different versions
 *     of the rotate kernel with the driver by calling the
 *     add_rotate_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_rotate_functions() 
{
    //add_rotate_function(&naive_rotate, naive_rotate_descr);   
    add_rotate_function(&rotate, rotate_descr);   

    //add_rotate_function(&attempt_two, rotate_two_descr);   
    //add_rotate_function(&attempt_three, rotate_three_descr);   
    //add_rotate_function(&attempt_four, rotate_four_descr);   
    //add_rotate_function(&attempt_five, rotate_five_descr);   
    //add_rotate_function(&attempt_six, rotate_six_descr);   
    //add_rotate_function(&attempt_seven, rotate_seven_descr);   
    //add_rotate_function(&attempt_eight, rotate_eight_descr);   
    //add_rotate_function(&attempt_nine, rotate_nine_descr);   
    //add_rotate_function(&attempt_ten, rotate_ten_descr);   
    //add_rotate_function(&attempt_eleven, rotate_eleven_descr);   

    /* ... Register additional rotate functions here */
}

