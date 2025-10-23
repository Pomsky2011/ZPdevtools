// Test struct member access in assignment context
struct Point {
    int x;
    int y;
};

int main() {
    struct Point p;
    int a;
    a = p.x;
    return a;
}
