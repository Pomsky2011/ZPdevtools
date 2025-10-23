// Test compound assignment operators

int main() {
    int x = 10;
    int y = 5;

    // Test +=
    x += y;  // x = 15

    // Test -=
    x -= 3;  // x = 12

    // Test *=
    x *= 2;  // x = 24

    // Test /=
    x /= 4;  // x = 6

    // Test %=
    x %= 5;  // x = 1

    // Test &=
    y &= 3;  // y = 1 (5 & 3 = 1)

    // Test |=
    y |= 4;  // y = 5 (1 | 4 = 5)

    // Test ^=
    y ^= 1;  // y = 4 (5 ^ 1 = 4)

    return x + y;  // Should return 5
}
