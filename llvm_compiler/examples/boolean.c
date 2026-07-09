// Modern C style (bool + // comments) also works through zpcc,
// since clang is the front end.
#include <stdbool.h>

bool is_even(int n) {
    return (n & 1) == 0;
}

int count_evens(int *arr, int len) {
    int count = 0;
    for (int i = 0; i < len; i++) {
        if (is_even(arr[i])) count++;
    }
    return count;
}
