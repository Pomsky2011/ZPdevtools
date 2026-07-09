int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
int sum_to(int n) {
    int s = 0;
    for (int i = 1; i <= n; i++) s += i;
    return s;
}
int main() {
    return factorial(5) + sum_to(4);   // 120 + 10 = 130
}
