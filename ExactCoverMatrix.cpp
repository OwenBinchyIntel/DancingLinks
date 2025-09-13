#include <chrono>
#include <iostream>

#include "ConstraintMatrix.hpp"

namespace {
const int g_InitialBoard[6][7] {
    { 0, 0, 1, 0, 1, 1, 0},
    { 1, 0, 0, 1, 0, 0, 1},
    { 0, 1, 1, 0, 0, 1, 0},
    { 1, 0, 0, 1, 0, 0, 0},
    { 0, 1, 0, 0, 0, 0, 1},
    { 0, 0, 0, 1, 1, 0, 1}
};
const std::vector<std::vector<int>> bIx {
    { 2, 4, 5 },
    { 0, 3, 6 },
    { 1, 2, 5 },
    { 0, 3 },
    { 1, 6 },
    { 3, 4, 6 }
};

const std::vector<std::vector<char>> bChar {
    { 'C', 'E', 'F' },
    { 'A', 'D', 'G' },
    { 'B', 'C', 'F' },
    { 'A', 'D' },
    { 'B', 'G' },
    { 'D', 'E', 'G' }
};

// A B C D E F G
// C E F
// A D G
// B C F
// A D 
// B G 
// D E G

// std::size_t t_Constraints, std::size_t t_Possibilities, std::size_t t_MaxConstraintsPerPossibility
ConstraintMatrix<7, 18> g_ConstraintMatrix;

}

int main() {

    for (const auto& constraints : bIx) {
        g_ConstraintMatrix.AddPossibility(constraints);
    }

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_ConstraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
