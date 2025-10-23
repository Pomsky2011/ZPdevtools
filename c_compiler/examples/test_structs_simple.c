// Test struct support - simplified version

struct Point {
    int x;
    int y;
};

int get_x(struct Point p) {
    return p.x;
}

int main() {
    struct Point p;
    int val = get_x(p);
    return val;
}
