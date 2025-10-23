// Test goto and labels
// goto allows jumping to labeled statements

int main() {
    int i = 0;
    int sum = 0;

    // Using goto to create a loop (old-school style)
loop_start:
    sum = sum + i;
    i = i + 1;

    if (i < 5) {
        goto loop_start;  // Jump back to label
    }

    // Skip a section with goto
    if (sum > 0) {
        goto skip_section;
    }

    sum = 999;  // This won't execute

skip_section:
    sum = sum + 100;  // sum = 10 + 100 = 110

    return sum;  // Returns 110
}
