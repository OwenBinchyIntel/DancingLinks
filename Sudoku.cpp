#include <algorithm>
#include <chrono>
#include <iostream>

#include "ConstraintMatrix.hpp"

namespace {

//          RC -------------------------    R# -------------------------    C# -------------------------
//          0       1       2       3       4       5       6       7       8       9       10      11  
//          R1C1    R1C2    R2C1    R2C2    R1#1    R1#2    R2#1    R2#2    C1#1    C1#2    C2#1    C2#2
// R1C1#1   1       0       0       0       1       0       0       0       1       0       0       0
// R1C1#2   1       0       0       0       0       1       0       0       0       1       0       0
// R1C2#1   0       1       0       0       1       0       0       0       0       0       1       0
// R1C2#2   0       1       0       0       0       1       0       0       0       0       0       1
// R2C1#1   0       0       1       0       0       0       1       0       1       0       0       0
// R2C1#2   0       0       1       0       0       0       0       1       0       1       0       0
// R2C2#1   0       0       0       1       0       0       1       0       0       0       1       0
// R2C2#2   0       0       0       1       0       0       0       1       0       0       0       1


// initial_board = [
//     [5, 3, 0, 0, 7, 0, 0, 0, 0],
//     [6, 0, 0, 1, 9, 5, 0, 0, 0],
//     [0, 9, 8, 0, 0, 0, 0, 6, 0],
//     [8, 0, 0, 0, 6, 0, 0, 0, 3],
//     [4, 0, 0, 8, 0, 3, 0, 0, 1],
//     [7, 0, 0, 0, 2, 0, 0, 0, 6],
//     [0, 6, 0, 0, 0, 0, 2, 8, 0],
//     [0, 0, 0, 4, 1, 9, 0, 0, 5],
//     [0, 0, 0, 0, 8, 0, 0, 7, 9]
// ]

constexpr int g_InitialBoard[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0}, {6, 0, 0, 1, 9, 5, 0, 0, 0}, {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {8, 0, 0, 0, 6, 0, 0, 0, 3}, {4, 0, 0, 8, 0, 3, 0, 0, 1}, {7, 0, 0, 0, 2, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0}, {0, 0, 0, 4, 1, 9, 0, 0, 5}, {0, 0, 0, 0, 8, 0, 0, 7, 9}};
constexpr std::size_t g_NumInitialValues = 30;

constexpr int g_SWidth = 3;
constexpr int g_Width = 9;

int ConstraintIxCell(int row, int col) {
    return row * g_Width + col;
}
int ConstraintIxRow(int col, int val) {
    constexpr int offset = g_Width * g_Width;
    return offset + (col * g_Width) + val;
}
int ConstraintIxCol(int row, int val) {
    constexpr int offset = g_Width * g_Width * 2;
    return offset + (row * g_Width) + val;
}
int ConstraintIxSqu(int row, int col, int val) {
    constexpr int offset = g_Width * g_Width * 3;
    int sqix = ((row / g_SWidth) * g_SWidth) + (col / g_SWidth);
    return offset + (sqix * g_Width) + val;
}

void PrintSolution(std::vector<std::vector<int>> selections) {
    int board[9][9];
    for (auto &selection : selections) {
        // Figure out this possibility's row/col/val

        std::sort(selection.begin(), selection.end());

        int row = selection[0] / 9;
        int col = selection[0] % 9;
        int val = selection[1] % 9;
        assert(row < 9 && row >= 0);
        assert(col < 9 && col >= 0);
        assert(val < 9 && val >= 0);
        board[row][col] = val + 1;
    }

    for (int row = 0; row < 9; ++row) {
        if (row % 3 == 0 && row != 0) {
            std::cout << "------+------+------\n";
        }
        for (int col = 0; col < 9; ++col) {
            if (col % 3 == 0 && col != 0) {
                std::cout << "| ";
            }
            std::cout << board[row][col] << ' ';
        }
        std::cout << '\n';
    }
}

constexpr std::size_t g_NumConstraints = g_Width * g_Width * 4 + g_NumInitialValues;
constexpr std::size_t g_MaxNodes = g_Width * g_Width * g_Width * 5;
ConstraintMatrix<g_NumConstraints, g_MaxNodes> g_ConstraintMatrix;

} // namespace

int main() {
    for (int row = 0; row < g_Width; ++row) {
        for (int col = 0; col < g_Width; ++col) {
            for (int val = 0; val < g_Width; ++val) {
                // Cell
                // Row
                // Col
                // Square
                std::vector<int> constraints;
                constraints.push_back(ConstraintIxCell(row, col));
                constraints.push_back(ConstraintIxRow(col, val));
                constraints.push_back(ConstraintIxCol(row, val));
                constraints.push_back(ConstraintIxSqu(row, col, val));
                g_ConstraintMatrix.AddPossibility(constraints);
            }
        }
    }
#if 1
    // Initial values
    int initValIx = 0;
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            int val = g_InitialBoard[row][col];
            if (val != 0) {
                --val;
                constexpr int offset = 9 * 9 * 4;
                std::vector<int> constraints;
                constraints.push_back(ConstraintIxCell(row, col));
                constraints.push_back(ConstraintIxRow(col, val));
                constraints.push_back(ConstraintIxCol(row, val));
                constraints.push_back(ConstraintIxSqu(row, col, val));
                constraints.push_back(offset + initValIx);
                g_ConstraintMatrix.AddPossibility(constraints);
                ++initValIx;
            }
        }
    }
    assert(initValIx == g_NumInitialValues);
#endif

    g_ConstraintMatrix.SetPrintFunction(PrintSolution);

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_ConstraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
