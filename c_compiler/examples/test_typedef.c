// Test typedef declarations
// Typedef creates type aliases

typedef int myint;
typedef char* string;
typedef int arr10[10];

int main() {
    // Using typedefs is not yet supported in variable declarations
    // This test just verifies that typedef declarations parse correctly
    // and are documented in the output assembly

    int x = 42;
    return x;
}
