#include "math.h"

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int absolute(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}
