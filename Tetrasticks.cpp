#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

#include <map>
#include <unordered_set>

#include "ConstraintMatrix.hpp"

namespace {

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";

std::vector<std::string> g_Colors = {
    "\033[38;5;0m",  
    "\033[38;5;1m",  
    "\033[38;5;2m",  
    "\033[38;5;3m",  
    "\033[38;5;4m",  
    "\033[38;5;5m",
    "\033[38;5;6m",  
    "\033[38;5;7m",  
    "\033[38;5;8m",  
    "\033[38;5;9m",  
    "\033[38;5;10m", 
    "\033[38;5;11m",
    "\033[38;5;12m", 
    "\033[38;5;13m", 
    "\033[38;5;14m", 
    "\033[38;5;15m",
};

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

class Tetrastick {
public:
    // private:
    // char m_Label;
    std::unordered_set<Pos, PosHash> m_HorizontalSegments; // { {2, 3} , {3, 3}};
    std::unordered_set<Pos, PosHash> m_VerticalSegments;   // { {4, 3} , {4, 4} };
    std::unordered_set<Pos, PosHash> m_InteriorJunctions;  // { {3, 3} , {4, 4} };
    std::string m_Color;
    int ix;

    bool operator==(const Tetrastick &rhs) const {
        if (m_HorizontalSegments.size() != rhs.m_HorizontalSegments.size()) {
            return false;
        }
        if (m_VerticalSegments.size() != rhs.m_VerticalSegments.size()) {
            return false;
        }
        for (const auto &segment : m_HorizontalSegments) {
            if (!rhs.m_HorizontalSegments.contains(segment)) {
                return false;
            }
        }
        for (const auto &segment : m_VerticalSegments) {
            if (!rhs.m_VerticalSegments.contains(segment)) {
                return false;
            }
        }
        return true;
    }

    void GenerateInternalNodes() {
        m_InteriorJunctions.clear();
        for (const auto &segment : m_HorizontalSegments) {
            if (m_HorizontalSegments.contains({segment.x + 1, segment.y})) {
                m_InteriorJunctions.insert({segment.x + 1, segment.y});
            }
        }
        for (const auto &segment : m_VerticalSegments) {
            if (m_VerticalSegments.contains({segment.x, segment.y + 1})) {
                m_InteriorJunctions.insert({segment.x, segment.y + 1});
            }
        }
    }

    std::unordered_set<Pos, PosHash> Nodes() const {
        std::unordered_set<Pos, PosHash> nodes;
        for (const auto &segment : m_HorizontalSegments) {
            nodes.insert(segment);
            nodes.insert({segment.x + 1, segment.y});
        }
        for (const auto &segment : m_VerticalSegments) {
            nodes.insert(segment);
            nodes.insert({segment.x, segment.y + 1});
        }
        return nodes;
    }

    void Print() const {
        constexpr int width = 6, height = 6;
        for (int y = height - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                Pos pos(x, y);
                if (y < height - 1) {
                    if (m_VerticalSegments.contains(pos)) {
                        // std::cout << '|';
                        std::cout << m_Color;
                    } else {
                        // std::cout << ' ';
                        std::cout << RESET;
                    }
                    std::cout << '|';
                } else {
                    std::cout << ' ';
                }
                if (x < width - 1) {
                    if (m_HorizontalSegments.contains(pos)) {
                        // std::cout << '_';
                        std::cout << m_Color;
                    } else {
                        // std::cout << ' ';
                        std::cout << RESET;
                    }
                    std::cout << '_';
                } else {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
        std::cout << RESET;
        for (const auto &jun : m_InteriorJunctions) {
            std::cout << jun.x << ' ' << jun.y << '\t';
        }
        std::cout << '\n';
    }

    void Normalize() {
        // Find leftmost and lowermost segmants
        int leftmost = 999, lowermost = 999;
        for (const auto &segment : m_HorizontalSegments) {
            leftmost = std::min(leftmost, segment.x);
            lowermost = std::min(lowermost, segment.y);
        }
        for (const auto &segment : m_VerticalSegments) {
            leftmost = std::min(leftmost, segment.x);
            lowermost = std::min(lowermost, segment.y);
        }
        // Should be redundant..
        for (const auto &segment : m_InteriorJunctions) {
            leftmost = std::min(leftmost, segment.x);
            lowermost = std::min(lowermost, segment.y);
        }
        Pos offset{-leftmost, -lowermost};
        Move(offset);
    }
    void Move(Pos offset) {
        std::unordered_set<Pos, PosHash> movedSegmants;
        for (auto &segment : m_HorizontalSegments) {
            movedSegmants.insert(segment + offset);
        }
        m_HorizontalSegments = std::move(movedSegmants);
        for (auto &segment : m_VerticalSegments) {
            movedSegmants.insert(segment + offset);
        }
        m_VerticalSegments = std::move(movedSegmants);
        for (auto &segment : m_InteriorJunctions) {
            movedSegmants.insert(segment + offset);
        }
        m_InteriorJunctions = std::move(movedSegmants);
    }
    void Rotate90Clockwise() {
        std::unordered_set<Pos, PosHash> horiSegmants;
        for (auto &segment : m_VerticalSegments) {
            horiSegmants.insert({segment.y, -segment.x});
        }
        std::unordered_set<Pos, PosHash> vertSegmants;
        for (auto &segment : m_HorizontalSegments) {
            vertSegmants.insert({segment.y, -segment.x - 1});
        }
        std::unordered_set<Pos, PosHash> intSegmants;
        for (auto &segment : m_InteriorJunctions) {
            intSegmants.insert({segment.y, -segment.x});
        }

        m_HorizontalSegments = std::move(horiSegmants);
        m_VerticalSegments = std::move(vertSegmants);
        m_InteriorJunctions = std::move(intSegmants);

        Normalize();
    }
    void FlipHorizontal() {
        std::unordered_set<Pos, PosHash> movedSegmants;
        for (auto &segment : m_HorizontalSegments) {
            movedSegmants.insert({-segment.x - 1, segment.y});
        }
        m_HorizontalSegments = std::move(movedSegmants);
        for (auto &segment : m_VerticalSegments) {
            movedSegmants.insert({-segment.x, segment.y});
        }
        m_VerticalSegments = std::move(movedSegmants);
        for (auto &segment : m_InteriorJunctions) {
            movedSegmants.insert({-segment.x, segment.y});
        }
        m_InteriorJunctions = std::move(movedSegmants);
        Normalize();
    }
};

struct TetrastickHash {
    std::size_t operator()(const Tetrastick &t) const {
        std::size_t hash;
        for (const auto &segment : t.m_HorizontalSegments) {
            hash |= PosHash()(segment);
        }
        for (const auto &segment : t.m_VerticalSegments) {
            hash |= PosHash()(segment);
        }
        return hash;
    }
};

void Print(const std::vector<Tetrastick> &sticks) {
    constexpr int width = 6, height = 6;
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            std::cout << RESET;
            Pos pos(x, y);
            bool vertical = false, horizontal = false;
            if (y < height - 1) {
                for (const auto &stick : sticks) {
                    if (stick.m_VerticalSegments.contains(pos)) {
                        std::cout << stick.m_Color;
                        assert(!vertical);
                        vertical = true;
                        // break;
                    }
                }
                std::cout << '|';
            } else {
                std::cout << ' ';
            }
            std::cout << RESET;
            if (x < width - 1) {
                for (const auto &stick : sticks) {
                    if (stick.m_HorizontalSegments.contains(pos)) {
                        std::cout << stick.m_Color;
                        assert(!horizontal);
                        horizontal = true;
                        // break;
                    }
                }
                std::cout << '_';
            } else {
                std::cout << ' ';
            }
            std::cout << RESET;
        }
        std::cout << '\n';
    }
    std::cout << RESET;
}

std::unordered_set<Tetrastick, TetrastickHash> g_FreeTetrasticks;

enum Direction {
    UP,
    LEFT,
    DOWN,
    RIGHT,
};

bool SetContainsTransform(Tetrastick tetrastick) {
    tetrastick.Normalize();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }

    tetrastick.Rotate90Clockwise();
    tetrastick.FlipHorizontal();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }
    tetrastick.Rotate90Clockwise();
    if (g_FreeTetrasticks.contains(tetrastick)) {
        return true;
    }

    return false;
}

void AddSticks(Tetrastick tetrastick, int count) {
    if (count <= 0) {
        assert(tetrastick.m_HorizontalSegments.size() + tetrastick.m_VerticalSegments.size() == 4);
        if (!SetContainsTransform(tetrastick)) {
            tetrastick.Normalize();
            tetrastick.GenerateInternalNodes();
            tetrastick.m_Color = g_Colors[g_FreeTetrasticks.size()];
            tetrastick.ix = int(g_FreeTetrasticks.size());
            // std::cout << " ==== \n";
            // std::cout << g_FreeTetrasticks.size() << '\n';
            // tetrastick.Print();
            // std::cout << std::flush;
            g_FreeTetrasticks.insert(tetrastick);
        }
    } else {
        for (auto node : tetrastick.Nodes()) {
            // Attach vertical
            if (!tetrastick.m_VerticalSegments.contains(node)) {
                tetrastick.m_VerticalSegments.insert(node);
                AddSticks(tetrastick, count - 1);
                tetrastick.m_VerticalSegments.extract(node);
            }
            Pos node2 = {node.x, node.y - 1};
            if (!tetrastick.m_VerticalSegments.contains(node2)) {
                tetrastick.m_VerticalSegments.insert(node2);
                AddSticks(tetrastick, count - 1);
                tetrastick.m_VerticalSegments.extract(node2);
            }

            // Attach horizontal
            if (!tetrastick.m_HorizontalSegments.contains(node)) {
                tetrastick.m_HorizontalSegments.insert(node);
                AddSticks(tetrastick, count - 1);
                tetrastick.m_HorizontalSegments.extract(node);
            }
            node = {node.x - 1, node.y};
            if (!tetrastick.m_HorizontalSegments.contains(node)) {
                tetrastick.m_HorizontalSegments.insert(node);
                AddSticks(tetrastick, count - 1);
                tetrastick.m_HorizontalSegments.extract(node);
            }
        }
    }
}

void GenerateTetrasticks() {
    // Place stick 1
    Tetrastick tetrastick;
    tetrastick.m_VerticalSegments.insert({0, 0});
    AddSticks(tetrastick, 3);
}

int ConstraintIndexH(Pos segment) {
    constexpr int offset = 15;
    return offset + segment.x + segment.y * 5;
}

int ConstraintIndexV(Pos segment) {
    constexpr int offset = 15 + 5 * 6;
    return offset + segment.x + segment.y * 6;
}

int ConstraintIndexI(Pos segment) {
    constexpr int offset = 15 + 5 * 6 + 6 * 5;
    return offset + (segment.x - 1) + (segment.y - 1) * 4;
}

ConstraintMatrix<75, 10000, 16> g_ConstraintMatrix;

void PrintFunction(std::vector<std::vector<int>> selections) {
    std::array<std::array<int, 6>, 5> h;
    std::array<std::array<int, 5>, 6> v;
    for (auto &selection : selections) {
        std::sort(selection.begin(), selection.end());

        const int &pieceIx = selection[0];
        for (auto cix = selection.begin() + 1; cix != selection.end(); ++cix) {
            if (*cix < (15 + 5 * 6)) {
                int hix = *cix - 15;
                int x = hix % 5;
                int y = hix / 5;
                assert(x < 5 && y < 6);
                h[x][y] = pieceIx;
            } else if (*cix < (15 + 5 * 6 + 5 * 6)) {
                int vix = *cix - (15 + 5 * 6);
                int x = vix % 6;
                int y = vix / 6;
                assert(x < 6 && y < 5);
                v[x][y] = pieceIx;
            } else {
                break;
            }
        }
    }

    for (int y = 5; y >= 0; --y) {
        for (int x = 0; x < 6; ++x) {
            if (y < 5) {
                assert(v[x][y] < g_Colors.size());
                std::cout << g_Colors[v[x][y]] << '|';
            } else {
                std::cout << ' ';
            }
            std::cout << RESET;
            if (x < 5) {
                assert(h[x][y] < g_Colors.size());
                std::cout << g_Colors[h[x][y]] << '_';
            } else {
                std::cout << ' ';
            }
            std::cout << RESET;
        }
        std::cout << '\n';
    }

    // We only want to print a few solutions.
    static int count = 5;
    if (--count < 0) {
        g_ConstraintMatrix.SetPrintFunction(nullptr);
    }
}


// 1, 3, 7, 10, 11
int g_RemovedStick = 10;

} // namespace

int main() {
    // Find all the tetrasticks
    GenerateTetrasticks();
    std::cout << "Found " << g_FreeTetrasticks.size() << " free tetrasticks.\n";

    std::unordered_set<Tetrastick, TetrastickHash> fixedTetrasticks;
    for (auto tstick : g_FreeTetrasticks) {
        if (tstick.ix == g_RemovedStick) {
            std::cout << "Removing stick : " << g_RemovedStick << '\n';
            tstick.Print();
            continue;
        }

        for (int i = 0; i < 4; ++i) {
            if (!fixedTetrasticks.contains(tstick)) {
                fixedTetrasticks.insert(tstick);
            }
            tstick.Rotate90Clockwise();
        }
        tstick.FlipHorizontal();
        for (int i = 0; i < 4; ++i) {
            if (!fixedTetrasticks.contains(tstick)) {
                fixedTetrasticks.insert(tstick);
            }
            tstick.Rotate90Clockwise();
        }
    }

    std::cout << "There are " << fixedTetrasticks.size() << " fixed tetrasticks.\n";

    for (int x = 0; x < 6; ++x) {
        for (int y = 0; y < 6; ++y) {
            for (auto tstick : fixedTetrasticks) {
                // {
                // auto tstick = *fixedTetrasticks.begin();
                Pos offset{x, y};
                tstick.Move(offset);
                bool fits = true;
                for (const auto &segment : tstick.m_HorizontalSegments) {
                    if (segment.x > 4 || segment.y > 5) {
                        fits = false;
                        break;
                    }
                }
                for (const auto &segment : tstick.m_VerticalSegments) {
                    if (segment.x > 5 || segment.y > 4) {
                        fits = false;
                        break;
                    }
                }
                if (fits) {
                    // std::cout << "=====\n";
                    // tstick.Print();
                    // std::cout << std::flush;
                    std::vector<int> constraints;
                    for (const auto &segment : tstick.m_HorizontalSegments) {
                        constraints.push_back(ConstraintIndexH(segment));
                    }
                    for (const auto &segment : tstick.m_VerticalSegments) {
                        constraints.push_back(ConstraintIndexV(segment));
                    }
                    for (const auto &segment : tstick.m_InteriorJunctions) {
                        if (segment.x == 0 || segment.x > 4 || segment.y == 0 || segment.y > 4) {
                        } else {
                            constraints.push_back(ConstraintIndexI(segment));
                        }
                    }
                    constraints.push_back(tstick.ix > g_RemovedStick ? tstick.ix - 1 : tstick.ix);
                    g_ConstraintMatrix.AddPossibility(constraints);
                }
            }
        }
    }

    g_ConstraintMatrix.SetPrintFunction(PrintFunction);

    // Solve
    const auto time_s = std::chrono::high_resolution_clock::now();
    int solutions = g_ConstraintMatrix.Solutions();
    const auto time_e = std::chrono::high_resolution_clock::now();
    const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_e - time_s).count();
    std::cout << "Found " << solutions << " possible solutions in " << time_ms << "ms\n";

    return 0;
}
