// Test ternary operator (? :)

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

int abs_value(int x) {
    return (x < 0) ? -x : x;
}

int main() {
    int a = 10;
    int b = 20;

    int maximum = max(a, b);   // Should be 20
    int minimum = min(a, b);   // Should be 10

    int neg = -5;
    int absolute = abs_value(neg);  // Should be 5

    // Nested ternary
    int result = (a > b) ? a : (b > a) ? b : 0;

    return maximum + minimum + absolute;
}
