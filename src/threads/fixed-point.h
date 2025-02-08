#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <stdint.h>

#define F (1<<14)

int N_to_FP(int n);
int FP_to_N(int x);
int FP_to_N_nearest(int x);
int add_FP(int x, int y);
int sub_FP(int x, int y);
int add_FP_to_N(int x, int n);
int sub_FP_from_N(int n, int x);
int mul_FP(int x, int y);
int mul_FP_to_N(int x, int n);
int div_FP(int x, int y);
int div_FP_by_N(int x, int n);

int
N_to_FP(int n)
{
    return n*F;
}

int
FP_to_N(int x)
{
    return x/F;
}

int
FP_to_N_nearest(int x)
{
    if(x>=0) return (x+F/2)/F; // 반올림
    else return (x-F/2)/F;
}

int
add_FP(int x, int y)
{
    return x+y;
}

int
sub_FP(int x, int y)
{
    return x-y;
}

int
add_FP_to_N(int x, int n)
{
    return x+n*F;
}

int
sub_FP_from_N(int n, int x)
{
    return x-n*F;
}

int
mul_FP(int x, int y)
{
    return ((int64_t)x)*y/F;
}

int 
mul_FP_to_N(int x, int n)
{
    return x*n;
}

int 
div_FP(int x, int y)
{
    return ((int64_t)x)*F/y; 
}

int
div_FP_by_N(int x, int n)
{
    return x/n; 
}

#endif
