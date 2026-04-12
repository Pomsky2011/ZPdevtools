// Simple addition function
int add(int a, int b) {
    return a + b;
}

// Factorial function
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

// Main function
int main() {
    int x;
    int y;
    int result;

    x = 5;
    y = 10;
    result = add(x, y);

    return result;
}
