// Test break and continue

int main() {
    int sum = 0;
    int i;

    // Test break - sum first 5 numbers only
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            break;
        }
        sum = sum + i;
    }
    // sum should be 0+1+2+3+4 = 10

    // Test continue - skip even numbers
    for (i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            continue;
        }
        sum = sum + i;
    }
    // sum should be 10 + 1+3+5+7+9 = 35

    return sum;
}
