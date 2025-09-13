#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "ConstraintMatrix.hpp"

namespace {
struct Pos {
    int x, y;

    constexpr Pos operator+(const Pos &rhs) const { return {x + rhs.x, y + rhs.y}; }
    constexpr bool operator==(const Pos &rhs) const { return x == rhs.x && y == rhs.y; }
};
struct PosHash {
    std::size_t operator()(const Pos &p) const {
        // Simple hash function combining x and y
        std::size_t hashX = std::hash<int>{}(p.x);
        std::size_t hashY = std::hash<int>{}(p.y);

        // Combine the hash values
        return hashX ^ (hashY + 0x9e3779b9 + (hashX << 6) + (hashX >> 2));
    }
};

class Pentomino {
public:
    Pentomino()
        : m_Label{0}
        , m_Cells{} {};
    constexpr Pentomino(char label, std::initializer_list<Pos> l) noexcept
        : m_Label{label}
        , m_Cells{} {
        auto it = l.begin();
        for (int i = 0; i < 5; ++i) {
            m_Cells[i] = *it++;
        }
        Normalize();
    }

    constexpr void FlipHorizontal() {
        int max_x = std::numeric_limits<int>::min();
        for (const auto &cell : m_Cells) {
            if (cell.x > max_x) {
                max_x = cell.x;
            }
        }
        for (auto &cell : m_Cells) {
            cell.x = max_x - cell.x;
        }
        Normalize();
    }

    constexpr void FlipVertical() {
        int max_y = std::numeric_limits<int>::min();
        for (const auto &cell : m_Cells) {
            if (cell.x > max_y) {
                max_y = cell.x;
            }
        }
        for (auto &cell : m_Cells) {
            cell.y = max_y - cell.y;
        }
        Normalize();
    }

    constexpr void RotateClockwise() {
        int max_x = std::numeric_limits<int>::min();
        int max_y = std::numeric_limits<int>::min();
        for (const auto &cell : m_Cells) {
            if (cell.x > max_x) {
                max_x = cell.x;
            }
            if (cell.y > max_y) {
                max_y = cell.y;
            }
        }
        for (auto &cell : m_Cells) {
            cell = {max_y - cell.y, cell.x};
        }
        Normalize();
    }

    constexpr void Normalize() {
        int min_x = std::numeric_limits<int>::max();
        int min_y = std::numeric_limits<int>::max();
        for (const auto &cell : m_Cells) {
            if (cell.x < min_x) {
                min_x = cell.x;
            }
            if (cell.y < min_y) {
                min_y = cell.y;
            }
        }
        for (auto &cell : m_Cells) {
            cell.x -= min_x;
            cell.y -= min_y;
        }

        // Now sort the cells:
        std::sort(m_Cells.begin(), m_Cells.end(),
                  [](const Pos &lhs, const Pos &rhs) { return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y); });
    }

    void Print() const {
        for (int j = 0; j < 5; ++j) {
            for (int i = 0; i < 5; ++i) {
                bool filled = false;
                for (int n = 0; n < 5; ++n) {
                    if (m_Cells[n].x == i && m_Cells[n].y == j) {
                        std::cout << 'X';
                        filled = true;
                        break;
                    }
                }
                if (!filled) {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
    }

    constexpr bool operator==(const Pentomino &rhs) const {
        for (int i = 0; i < 5; ++i) {
            if (m_Cells[i] != rhs.m_Cells[i]) {
                return false;
            }
        }
        return true;
    }

    const std::array<Pos, 5> &Cells() const { return m_Cells; }
    char Label() const { return m_Label; }

private:
    char m_Label;
    std::array<Pos, 5> m_Cells;
};

std::map<char, Pentomino> g_FreePentominos;
std::vector<Pentomino> g_OneSidedPentominos;
std::vector<Pentomino> g_FixedPentominos;

class Board {
public:
    static constexpr std::size_t Size = 8;

    Board() {
        for (int i = 0; i < Size; ++i) {
            for (int j = 0; j < Size; ++j) {
                if ((i == 3 || i == 4) && (j == 3 || j == 4)) {
                    // Middle 4 cells are missing.
                    continue;
                }

                // bool covered = false;
                // for (const auto& cell : g_FreePentominos['X'].Cells()) {
                //     Pos offset = {0, 1};
                //     if ((cell + offset) == Pos{i, j}) {
                //         covered = true;
                //         break;
                //     }
                // }
                // if (covered) {
                //     continue;
                // }

                m_Cells.insert(Pos{i, j});
            }
        }
    }

    void Print() const {
        std::cout << " -------- \n";
        for (int i = 0; i < Size; ++i) {
            std::cout << '|';
            for (int j = 0; j < Size; ++j) {
                if (m_Cells.contains(Pos(i, j))) {
                    std::cout << 'O';
                } else {
                    std::cout << 'X';
                }
            }
            std::cout << "|\n";
        }
        std::cout << " -------- \n";
    }

    bool Fits(const Pentomino &p, const Pos &offset = {0, 0}) {
        for (const auto &cell : p.Cells()) {
            if (!m_Cells.contains(cell + offset)) {
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_set<Pos, PosHash> m_Cells;
};


constexpr std::size_t k_NumLabels = 12;
constexpr std::array<char, k_NumLabels> labels = {'F', 'I', 'L', 'N', 'P', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
// constexpr std::size_t k_NumLabels = 11;
// constexpr std::array<char, k_NumLabels> labels = {'F', 'I', 'L', 'N', 'P', 'T', 'U', 'V', 'W', 'Y', 'Z'};

int ConstraintIx(const Pentomino &p) {
    int lix = 0;
    while (labels[lix] != p.Label()) {
        ++lix;
        assert(lix < k_NumLabels);
    }
    return lix + Board::Size * Board::Size;
}
int ConstraintIx(const Pos &p) {
    // return k_NumLabels + (p.x * Board::Size) + p.y;
    return (p.x * Board::Size) + p.y;
}

// 12 Pentominos
// 8*8 - 4 = 60 cells to fill
// 12 + 60 = 72 constraints
//      Leave in the removed 4 cells for now
// 1568 possible placements
ConstraintMatrix<76, 1568 * 6> g_constraintMatrix;

// 45 Y pentominos
// 1344 possible positions each
// Constraints are only that each tile must be filled (not that we must place each of the 45 pentominos, that is
// implied). 15x15 = 225 constraints 1344 = 1344 possibilities 1344 x 6 = 8064 nodes

// TODO - 5 not 6 right??

// ConstraintMatrix<225, 1344, 6> g_constraintMatrix;

} // namespace

int main() {

    // Populate Free Pentominos map
    g_FreePentominos['F'] = Pentomino('F', {{1, 0}, {2, 0}, {0, 1}, {1, 1}, {1, 2}});
    g_FreePentominos['I'] = Pentomino('I', {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}});
    g_FreePentominos['L'] = Pentomino('L', {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 3}});
    g_FreePentominos['N'] = Pentomino('N', {{1, 0}, {1, 1}, {0, 2}, {1, 2}, {0, 3}});
    g_FreePentominos['P'] = Pentomino('P', {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {0, 2}});
    g_FreePentominos['T'] = Pentomino('T', {{0, 0}, {1, 0}, {2, 0}, {1, 1}, {1, 2}});
    g_FreePentominos['U'] = Pentomino('U', {{0, 0}, {2, 0}, {0, 1}, {1, 1}, {2, 1}});
    g_FreePentominos['V'] = Pentomino('V', {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}});
    g_FreePentominos['W'] = Pentomino('W', {{0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2}});
    g_FreePentominos['X'] = Pentomino('X', {{0, 1}, {1, 0}, {1, 1}, {1, 2}, {2, 1}});
    g_FreePentominos['Y'] = Pentomino('Y', {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 2}});
    g_FreePentominos['Z'] = Pentomino('Z', {{0, 0}, {0, 1}, {1, 1}, {2, 1}, {2, 2}});

    std::cout << "There are " << g_FreePentominos.size() << " free Pentominos\n";

    // Rotate and generate One-Sided Pentominos
    for (const auto &entry : g_FreePentominos) {
        Pentomino p = entry.second;
        g_OneSidedPentominos.push_back(p);
        p.FlipHorizontal();
        bool distinct = true;
        for (int i = 0; i < 4; ++i) {
            if (p == entry.second) {
                distinct = false;
                break;
            }
            p.RotateClockwise();
        }
        if (distinct) {
            g_OneSidedPentominos.push_back(p);
        }
    }

    std::cout << "There are " << g_OneSidedPentominos.size() << " one-sided Pentominos\n";

    // Flip and generate Fixed Pentominos
    for (const auto &entry : g_OneSidedPentominos) {
        Pentomino p = entry;
        g_FixedPentominos.push_back(p);
        p.RotateClockwise();
        if (p == entry) {
            continue;
        }
        g_FixedPentominos.push_back(p);
        p.RotateClockwise();
        if (p == entry) {
            continue;
        }
        g_FixedPentominos.push_back(p);
        p.RotateClockwise();
        g_FixedPentominos.push_back(p);
    }

    std::cout << "There are " << g_FixedPentominos.size() << " fixed Pentominos\n";

    ///////////////////////////////////////////////////////////////////////////

    // Find all possible positions for pentominos to be placed on a board
    // Add them as possibilities to the contraint matrix

    Board board;
    board.Print();

    int possiblePositions = 0;
    const auto setup_start = std::chrono::high_resolution_clock::now();
    for (const auto &p : g_FixedPentominos) {
        if (p.Label() == 'X') {
            // continue;
            // for (const auto& offset : std::vector<Pos>{{0, 1}, {0, 2}, {1, 1}}) {
                // When X is at {1, 1}, we should assume 'P' is not flipped...

            // 'X' is at 23
            for (const auto& offset : std::vector<Pos>{{0, 1}}) {
            // 'X' is at 24
            // for (const auto& offset : std::vector<Pos>{{0, 2}}) {
            // 'X' is at 33
            // for (const auto& offset : std::vector<Pos>{{1, 1}}) {
                ++possiblePositions;
                std::vector<int> constraints { ConstraintIx(p) };
                // std::vector<int> constraints;
                for (const auto &cell : p.Cells()) {
                    constraints.push_back(ConstraintIx(cell + offset));
                }
                g_constraintMatrix.AddPossibility(constraints);
            }

            continue;
        }
        // Check if we fit into the board:
        for (int i = 0; i < Board::Size; ++i) {
            for (int j = 0; j < Board::Size; ++j) {
                Pos offset{i, j};
                if (board.Fits(p, offset)) {
                    ++possiblePositions;
                    std::vector<int> constraints { ConstraintIx(p) };
                    // std::vector<int> constraints;
                    for (const auto &cell : p.Cells()) {
                        constraints.push_back(ConstraintIx(cell + offset));
                    }
                    g_constraintMatrix.AddPossibility(constraints);
                }
            }
        }
    }
    // Remove middle squares from constraints (they don't have to be filled).
    g_constraintMatrix.RemoveConstraint(ConstraintIx(Pos{3, 3}));
    g_constraintMatrix.RemoveConstraint(ConstraintIx(Pos{3, 4}));
    g_constraintMatrix.RemoveConstraint(ConstraintIx(Pos{4, 3}));
    g_constraintMatrix.RemoveConstraint(ConstraintIx(Pos{4, 4}));
    // g_constraintMatrix.Cover();
    // Remove squares covered by 'X' from constraints.
    // for (const auto& cell : g_FreePentominos['X'].Cells()) {
    //     Pos offset = {0, 1};
    //     g_constraintMatrix.RemoveConstraint(ConstraintIx(cell + offset));
    // }

    const auto setup_end = std::chrono::high_resolution_clock::now();
    const auto setup_time_ms = std::chrono::duration_cast<std::chrono::microseconds>(setup_end - setup_start).count();
    std::cout << "Setup time : " << setup_time_ms << "us\n";
    std::cout << "Possible positions for fixed pentominos : " << possiblePositions << '\n';

    //////////////////////////////////////////////////////////////////////////

    // Find all solutions

    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_constraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
