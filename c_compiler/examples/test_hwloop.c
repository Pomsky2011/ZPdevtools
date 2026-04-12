// Test hardware LOOP/LPEND optimization
// This demonstrates the compiler using DEF88186's hardware loop instructions

int sum_array() {
    int sum;
    int i;

    sum = 0;

    // Simple counted loop - should use LOOP/LPEND
    for (i = 0; i < 100; i = i + 1) {
        sum = sum + i;
    }

    return sum;
}

int factorial_loop(int n) {
    int result;
    int i;

    result = 1;

    // This is a simple counted loop if n is constant at compile time
    // But since n is a parameter, it will use manual loop
    for (i = 0; i < n; i = i + 1) {
        result = result * i;
    }

    return result;
}

int main() {
    int x;
    int y;

    // Hardware loop example
    x = sum_array();

    // Manual loop example (parameter not constant)
    y = factorial_loop(10);

    return x;
}
