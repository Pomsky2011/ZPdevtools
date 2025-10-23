// Test switch/case statements

int get_day_type(int day) {
    int result = 0;

    switch (day) {
        case 1:
            result = 1;  // Monday
            break;

        case 2:
            result = 1;  // Tuesday
            break;

        case 6:
            result = 2;  // Saturday
            break;

        case 7:
            result = 2;  // Sunday
            break;

        default:
            result = 0;  // Invalid
            break;
    }

    return result;
}

int classify_number(int n) {
    int type = 0;

    switch (n) {
        case 0:
            type = 100;
            break;

        case 1:
            type = 101;
            break;

        case 2:
            type = 102;
            break;

        default:
            type = 999;
            break;
    }

    return type;
}

int main() {
    int weekday = get_day_type(3);      // Should be 1
    int weekend = get_day_type(7);      // Should be 2
    int invalid = get_day_type(10);     // Should be 0

    int zero = classify_number(0);      // Should be 100
    int one = classify_number(1);       // Should be 101
    int other = classify_number(99);    // Should be 999

    return weekday + weekend + invalid + zero + one + other;
}
