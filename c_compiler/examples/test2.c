// Test control flow
int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int sum_to_n(int n) {
    int sum;
    int i;

    sum = 0;
    i = 1;

    while (i <= n) {
        sum = sum + i;
        i = i + 1;
    }

    return sum;
}

int main() {
    int a;
    int b;

    a = max(10, 20);
    b = sum_to_n(10);

    return b;
}
