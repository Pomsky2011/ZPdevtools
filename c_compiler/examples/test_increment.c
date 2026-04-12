// Test increment and decrement operators

int main() {
    int x = 10;
    int y;

    // Pre-increment
    y = ++x;  // x = 11, y = 11

    // Post-increment
    y = x++;  // y = 11, x = 12

    // Pre-decrement
    y = --x;  // x = 11, y = 11

    // Post-decrement
    y = x--;  // y = 11, x = 10

    return x;  // Should return 10
}
