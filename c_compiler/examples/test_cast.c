// Test cast operators
// Cast between different types

int main() {
    int x = 300;              // Larger than 8-bit can hold
    char c = (char)x;         // Cast to char - truncates to 44 (300 & 0xFF)

    char small = 65;          // 'A'
    int large = (int)small;   // Cast to int - promotes to 16-bit

    // Pointer casts
    int value = 1000;
    int* ptr = &value;
    char* byte_ptr = (char*)ptr;  // Pointer cast (reinterpret)

    // Cast in expressions
    int result = (int)(c + 10);   // Cast result of addition

    return result + large;
}
