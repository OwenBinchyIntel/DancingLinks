#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>

#include "ConstraintMatrix.hpp"

namespace {

// #define WIDTH_9
#define WIDTH_4

#ifdef WIDTH_9
constexpr int k_SWidth = 3;
constexpr int k_Width = 9;
#endif // WIDTH_9
#ifdef WIDTH_4
constexpr int k_SWidth = 2;
constexpr int k_Width = 4;
#endif // WIDTH_4

class SudokuSolver {
public:
    using Board = std::vector<std::string>;
    SudokuSolver(const Board &board)
        : m_ConstraintMatrix(k_MinConstraints + CountInitialValues(board)) {
        SetEmptySudokuConstraints();
        SetInitialValueContraints(board);
        m_ConstraintMatrix.SetPrintFunction(PrintSolution);
    }

    int Solutions() { return m_ConstraintMatrix.Solutions(); }

private:
    void SetEmptySudokuConstraints() {
        // General sudoku contraints
        for (int row = 0; row < k_Width; ++row) {
            for (int col = 0; col < k_Width; ++col) {
                for (int val = 0; val < k_Width; ++val) {
                    // Cell
                    // Row
                    // Col
                    // Square
                    std::vector<int> constraints;
                    constraints.push_back(ConstraintIxCell(row, col));
                    constraints.push_back(ConstraintIxRow(col, val));
                    constraints.push_back(ConstraintIxCol(row, val));
                    constraints.push_back(ConstraintIxSqu(row, col, val));
                    m_ConstraintMatrix.AddPossibility(constraints);
                }
            }
        }
    }

    void SetInitialValueContraints(const Board &board) {
        int initValIx = 0;
        for (int row = 0; row < k_Width; ++row) {
            for (int col = 0; col < k_Width; ++col) {
                int val = BoardCharToVal(board[row][col]);
                if (val != 0) {
                    --val;
                    constexpr int offset = k_Width * k_Width * 4;
                    std::vector<int> constraints;
                    constraints.push_back(ConstraintIxCell(row, col));
                    constraints.push_back(ConstraintIxRow(col, val));
                    constraints.push_back(ConstraintIxCol(row, val));
                    constraints.push_back(ConstraintIxSqu(row, col, val));
                    constraints.push_back(offset + initValIx);
                    m_ConstraintMatrix.AddPossibility(constraints);
                    ++initValIx;
                }
            }
        }
        // assert(initValIx == numInitialValues);
    }

    static void PrintSolution(std::vector<std::vector<std::size_t>> selections) {
        char board[k_Width][k_Width];
        for (auto &selection : selections) {
            // Figure out this possibility's row/col/val

            std::sort(selection.begin(), selection.end());

            std::size_t row = selection[0] / k_Width;
            std::size_t col = selection[0] % k_Width;
            std::size_t val = selection[1] % k_Width;
            assert(row < k_Width && row >= 0);
            assert(col < k_Width && col >= 0);
            assert(val < k_Width && val >= 0);
            board[row][col] = BoardValToChar((int)val + 1);
        }
        for (int row = 0; row < k_Width; ++row) {
            if (row % k_SWidth == 0 && row != 0) {
                // std::cout << "------+-------+------\n";
            }
            for (int col = 0; col < k_Width; ++col) {
                if (col % 3 == 0 && col != 0) {
                    // std::cout << "| ";
                }
                std::cout << board[row][col];
                // std::cout << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }

    static std::size_t CountInitialValues(const Board &board) {
        std::size_t cnt = 0;
        for (const auto line : board) {
            for (char c : line) {
                cnt += c != '.' ? 1 : 0;
            }
        }
        return cnt;
    }

    static int ConstraintIxCell(int row, int col) { return row * k_Width + col; }
    static int ConstraintIxRow(int col, int val) {
        constexpr int offset = k_Width * k_Width;
        return offset + (col * k_Width) + val;
    }
    static int ConstraintIxCol(int row, int val) {
        constexpr int offset = k_Width * k_Width * 2;
        return offset + (row * k_Width) + val;
    }
    static int ConstraintIxSqu(int row, int col, int val) {
        constexpr int offset = k_Width * k_Width * 3;
        int sqix = ((row / k_SWidth) * k_SWidth) + (col / k_SWidth);
        return offset + (sqix * k_Width) + val;
    }
    static constexpr int BoardCharToVal(char c) {
        if (c == '.')
            return 0;
        return c - '0';
    }
    static constexpr char BoardValToChar(int c) { return c + '0'; }

private:
    static constexpr std::size_t k_MinConstraints = k_Width * k_Width * 4;
    static constexpr std::size_t k_MaxConstraints = k_MinConstraints + (k_Width * k_Width);
    static constexpr std::size_t k_MaxNodes = k_Width * k_Width * k_Width * 5;

    ConstraintMatrix<k_MaxConstraints, k_MaxNodes> m_ConstraintMatrix;
};

} // namespace

int main() {
    // clang-format off
    SudokuSolver::Board board = {
    #ifdef WIDTH_9
        "53..7....",
        "6..195...",
        ".98....6.",
        "8...6...3",
        "4..8.3..1",
        "7...2...6",
        ".6....28.",
        "...419..5",
        "....8..79",
    #endif // WIDTH_9
    #ifdef WIDTH_4
        "1234",
        "3412",
        "2...",
        "....",
    #endif // WIDTH_4
    }; // clang-format on

    SudokuSolver solver(board);

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = solver.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
