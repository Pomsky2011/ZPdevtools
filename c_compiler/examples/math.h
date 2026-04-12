#ifndef MATH_H
#define MATH_H

// Simple math library for DEF88186

#define PI 3
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

int add(int a, int b);
int subtract(int a, int b);
int multiply(int a, int b);
int absolute(int x);

#endif
