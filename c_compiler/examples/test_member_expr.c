// Test member access in different contexts
struct Point {
    int x;
};

int main() {
    struct Point p;
    p.x = 10;        // member assignment - works
    int a = 5 + 3;   // simple expression - should work
    return a;
}
