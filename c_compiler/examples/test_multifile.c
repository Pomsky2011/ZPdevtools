#include "math.h"

// Test macros
#define DEBUG 1
#define CIRCLE_AREA(r) (PI * (r) * (r))

int main() {
    int x = 10;
    int y = 5;
    int result;

    // Test function calls from included library
    result = add(x, y);  // 15
    result = subtract(x, y);  // 5
    result = multiply(x, y);  // 50

    // Test macros
    int max_val = MAX(x, y);  // 10
    int min_val = MIN(x, y);  // 5

    // Test conditional compilation
    #ifdef DEBUG
    result = absolute(-42);  // 42
    #endif

    // Test inline assembly
    __asm__("NOP");
    __asm__("LDA #$FF");

    // Calculate circle area using macro
    int radius = 5;
    int area = CIRCLE_AREA(radius);  // PI * 25 = 75

    return 0;
}
