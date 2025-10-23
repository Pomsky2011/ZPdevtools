// Test enum declarations
// Enums define named integer constants

enum Color {
    RED,
    GREEN,
    BLUE
};

enum Status {
    OK = 0,
    ERROR = -1,
    PENDING = 100
};

int main() {
    // Can use enum constants as integers
    int color = 0;    // RED
    int status = 0;   // OK

    return color + status;  // Returns 0
}
