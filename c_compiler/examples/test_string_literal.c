// Test string literals (basic support)
// Note: String literal support is basic in this compiler
// Strings are stored as char* pointers to .rodata section

int main() {
    // String literal - compiler will generate address
    // Full implementation would put this in .rodata section
    char* message = "Hello, World!";
    char* greeting = "Welcome";

    // For now, just demonstrate the feature compiles
    int result = 42;

    return result;
}
