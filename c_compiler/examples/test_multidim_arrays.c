// Test multi-dimensional array declaration
// Note: Full multi-dimensional subscripting (matrix[i][j])
// requires additional parser work. This demonstrates declaration.

int main() {
    // 2D array declaration - reserves space for 3x4 = 12 ints
    int matrix[3][4];

    // 3D array declaration - reserves space for 2x3x4 = 24 ints
    int cube[2][3][4];

    // The compiler allocates the correct amount of stack space
    // sizeof(matrix) would be 24 bytes (12 ints * 2 bytes)
    // sizeof(cube) would be 48 bytes (24 ints * 2 bytes)

    return 0;
}
