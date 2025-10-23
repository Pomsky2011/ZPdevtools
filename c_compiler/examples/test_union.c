// Test union support
union Data {
    int i;
    char c;
};

int main() {
    union Data d;
    d.i = 100;
    d.c = 'A';
    return d.i;
}
