// Test multiple variable declarations
// C89 allows declaring multiple variables of the same type in one statement

int main() {
    // Multiple declarations without initialization
    int a, b, c;

    // Multiple declarations with mixed initialization
    int x = 5, y = 10, z;

    // Initialize the uninitialized variables
    a = 1;
    b = 2;
    c = 3;
    z = 15;

    // Calculate sum
    int sum = a + b + c + x + y + z;  // 1 + 2 + 3 + 5 + 10 + 15 = 36

    return sum;  // Returns 36
}
