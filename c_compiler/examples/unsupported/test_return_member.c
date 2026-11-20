// Test returning struct member
struct Point {
    int x;
};

int main() {
    struct Point p;
    p.x = 42;
    int result = p.x;
    return result;
}
