// Test arrays - THIS WILL FAIL
int main() {
    int arr[10];
    int i;

    arr[0] = 42;
    arr[5] = arr[0] + 10;

    return arr[5];
}
