// Test do-while loop
// This loop executes at least once

int count_down(int n) {
    int i = n;
    do {
        i = i - 1;
    } while (i > 0);
    return i;
}

int main() {
    int result = count_down(5);  // Should return 0

    // Test with initial false condition (still executes once)
    int x = 10;
    do {
        x = x + 5;  // Executes once, x becomes 15
    } while (x < 10);  // False, but body already ran

    return result + x;  // 0 + 15 = 15
}
