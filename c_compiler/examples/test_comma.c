// Test comma operator
// Evaluates left to right, returns rightmost value

int main() {
    int a = 0;
    int b = 0;
    int c = 0;

    // Comma operator: (expr1, expr2, expr3) evaluates all, returns last
    int result = (a = 5, b = 10, c = 15);  // result = 15

    // Comma in for loop increment (common usage)
    int i = 0;
    int j = 10;
    for (i = 0; i < 5; i = i + 1, j = j - 1) {
        // i increments, j decrements
    }
    // After loop: i=5, j=5

    // Multiple side effects
    int x = (a++, b++, c++);  // All increment, x = old value of c (15)
    // Now: a=6, b=11, c=16, x=15

    return result + i + j + x;  // 15 + 5 + 5 + 15 = 40
}
