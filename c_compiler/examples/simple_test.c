#define VALUE 100

int add(int a, int b) {
    return a + b;
}

int main() {
    int x = VALUE;
    int y = 50;
    int result = add(x, y);

    // Inline assembly test
    __asm__("NOP");
    __asm__("LDA #$42");

    return result;
}
