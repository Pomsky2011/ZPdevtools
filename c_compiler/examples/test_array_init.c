// Test array initialization
// Arrays can be initialized with initialization lists

int main() {
    int arr[5] = {10, 20, 30, 40, 50};
    char str[4] = {'a', 'b', 'c', 'd'};

    // Access array elements
    int sum = arr[0] + arr[1] + arr[2];

    return sum;  // Should return 60
}
