// Test sizeof operator

int main() {
    int x;
    char c;
    int* ptr;

    // sizeof with types
    int size_int = sizeof(int);       // Should be 2
    int size_char = sizeof(char);     // Should be 1
    int size_ptr = sizeof(int*);      // Should be 2 (pointer)

    // sizeof with expressions
    int size_x = sizeof(x);           // Should be 2
    int size_c = sizeof(c);           // Should be 1

    return size_int + size_char + size_ptr + size_x + size_c;
}
