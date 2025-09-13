#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <utility>

#include "ConstraintMatrix.hpp"

namespace {

constexpr int k_BoardWidth = 15;
constexpr int k_Rows = k_BoardWidth;
constexpr int k_Cols = k_BoardWidth;
constexpr int k_Diags = (k_Rows + k_Cols - 1) * 2;
constexpr int k_Squares = k_Rows * k_Cols;

/*
We need to fill each row exactly once
We need to fill each column exactly once
We can only fill each diagonal at most once
*/

// constexpr int SquareIx(int row, int col) {
//     /* 4x4 board indexed as follows (in hex):
//         0123
//         4567
//         89AB
//         CDEF
//     */
//     return row * k_Cols + col;
// }
#if 0
constexpr int RowIx(int row) {
    return row;
}
constexpr int ColIx(int col) {
    return k_Rows + col;
}
constexpr std::pair<int, int> RowColFromIxs(int ix0, int ix1) {
    return {ix0, ix1 - k_Rows};
}
#else
// Organ-pipe ordering
// For N = 16
// R8 F8 R7 F7 R9 F9 . . . R0 F0
constexpr int RowIx(int row) {
    // 8 7 9 6 10 5 11 4 12 3 13 2 14 1 15 0
    int ix = 0;
    if (row >= k_Rows / 2) {
        ix = (row - (k_Rows / 2)) * 2;
    } else {
        ix = ((k_Rows / 2) - row) * 2 - 1;
    }
    return ix * 2;
}
constexpr int ColIx(int col) {
    // 8 7 9 6 10 5 11 4 12 3 13 2 14 1 15 0
    int ix = 0;
    if (col >= k_Cols / 2) {
        ix = (col - (k_Cols / 2)) * 2;
    } else {
        ix = ((k_Cols / 2) - col) * 2 - 1;
    }
    return ix * 2 + 1;
}
constexpr std::pair<int, int> RowColFromIxs(int ix0, int ix1) {
    // TODO - this is wrong, but will only break if we print solutions.
    return {ix0, ix1 - k_Rows};
}
#endif
constexpr int DiagIxP(int row, int col) {
    // Positive sloped diagonals
    return k_Rows + k_Cols + row + col;
}
constexpr int DiagIxN(int row, int col) {
    // Negative sloped diagonals
    return k_Rows + k_Cols + k_Diags/2 + (k_Rows-row - 1) + col;
}

void PrintFunction(std::vector<std::vector<int>> selections) {
    std::cout << "Solution found :\n";
    std::array<std::array<bool, k_Cols>, k_Rows> board { false };
    for (auto & selection : selections) {
        std::sort(selection.begin(), selection.end());
        // int row = selection[0];
        // int col = selection[1] - k_Rows;
        auto [row, col] = RowColFromIxs(selection[0], selection[1]);
        board[row][col] = true;
    }
    for (int row = k_Rows - 1; row >= 0; --row) {
        for (int col = 0; col < k_Cols; ++col) {
            std::cout << (board[row][col] ? 'Q' : '.');
        }
        std::cout << '\n';
    }
    std::cout << '\n';
}

ConstraintMatrix<k_Rows + k_Cols, k_Squares * 4, k_Diags> g_ConstraintMatrix;

} // namespace

int main() {

    // Populate constraint matrix
    for (int row = 0; row < k_Rows; ++row) {
        for (int col = 0; col < k_Cols; ++col) {
            g_ConstraintMatrix.AddPossibility({RowIx(row), ColIx(col), DiagIxP(row, col), DiagIxN(row, col)});
        }
    }

    // g_ConstraintMatrix.SetPrintFunction(PrintFunction);

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_ConstraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
