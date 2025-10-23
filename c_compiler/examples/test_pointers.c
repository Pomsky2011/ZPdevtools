// Test pointer support

int main() {
    int x = 42;
    int *ptr;

    // Get address of x
    ptr = &x;

    // Dereference pointer
    int y = *ptr;

    return y;
}
