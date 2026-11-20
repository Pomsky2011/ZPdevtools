// Test union member access
union Data {
    int i;
    char c;
};

int main() {
    union Data d;
    int x;
    x = d.i;
    return x;
}
