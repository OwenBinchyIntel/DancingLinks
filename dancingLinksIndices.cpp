#include <array>
#include <iostream>
#include <string>

#include <chrono>
#include <vector>

class ConstraintMatrix {
private:
    static constexpr int k_MaxWidth = 9;
    static constexpr int k_MaxPosibilities{k_MaxWidth * k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxRCConstraints{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxRNConstriants{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxCNConstraints{k_MaxWidth * k_MaxWidth};
    static constexpr int k_MaxConstraints{k_MaxRCConstraints + k_MaxRNConstriants + k_MaxCNConstraints};
    static constexpr int k_MaxNodes = k_MaxConstraints * k_MaxWidth;
    static constexpr int k_MaxSelections = k_MaxWidth * k_MaxWidth;

public:
    struct Header;
    struct Node {
        size_t left{0};
        size_t right{0};
        size_t up{0};
        size_t down{0};

        size_t col{0};
    };

    struct Header {
        int count;
    };

    ConstraintMatrix(int width)
        : m_Width{width} {
        ConnectColHeaders();
        PopulateGraph();
    }

    size_t &right(size_t n) { return m_Nodes[n].right; }
    size_t &left(size_t n) { return m_Nodes[n].left; }
    size_t &up(size_t n) { return m_Nodes[n].up; }
    size_t &down(size_t n) { return m_Nodes[n].down; }
    size_t &col(size_t n) { return m_Nodes[n].col; }
    int &count(size_t c) { return m_Headers[c - 1].count; }

    int Solutions() {
        if (right(m_RootNode) == m_RootNode) {
            return 1;
        }

        int lowestPossibilities = k_MaxPosibilities + 1;
        size_t lowestCol = 0;

        // Find constraint with fewest possibilities
        for (size_t colH = right(m_RootNode); colH != m_RootNode; colH = right(colH)) {
            if (count(colH) < lowestPossibilities) {
                lowestPossibilities = count(colH);
                lowestCol = colH;
            }
        }

        if (lowestPossibilities == 0) {
            // Unsatisfiable
            return 0;
        }

        // Consider constraint satisfied and iterate through its possibilities.
        Cover(lowestCol);
        int solutions = 0;
        for (size_t r = down(lowestCol); r != lowestCol; r = down(r)) {
            Select(r);
            solutions += Solutions();
            UnSelect(r);
        }

        UnCover(lowestCol);
        return solutions;
    }

    bool Set(int r, int c, int num) {
        --num;
        if (r < 0 || r >= m_Width || c < 0 || c >= m_Width || num < 0 || num >= m_Width) {
            return false;
        }
        size_t rc = 1 + (r * m_Width) + c;
        size_t rn = 1 + (m_Width * m_Width) + (r * m_Width) + num;

        for (size_t h = right(m_RootNode); h != m_RootNode; h = right(h)) {
            if (h == rc) {
                for (size_t n = down(h); n != h; n = down(n)) {
                    if (col(right(n)) == rn) {
                        Cover(col(n));
                        Select(n);
                        return true;
                    }
                }
            }
        }
        return false;
    }

private:
    void Cover(size_t c) {
        // Remove c from header list
        left(right(c)) = left(c);
        right(left(c)) = right(c);
        // Remove all rows from c from other columns that they are in
        for (size_t i = down(c); i != c; i = down(i)) {
            for (size_t j = right(i); j != i; j = right(j)) {
                down(up(j)) = down(j);
                up(down(j)) = up(j);
                --count(col(j));
            }
        }
    }

    void UnCover(size_t c) {
        // Reverse operation of cover
        for (size_t i = up(c); i != c; i = up(i)) {
            for (size_t j = left(i); j != i; j = left(j)) {
                ++count(col(j));
                up(down(j)) = j;
                down(up(j)) = j;
            }
        }
        right(left(c)) = c;
        left(right(c)) = c;
    }

    void Select(size_t n) {
        for (size_t j = right(n); j != n; j = right(j)) {
            Cover(col(j));
        }
    }

    void UnSelect(size_t n) {
        for (size_t j = left(n); j != n; j = left(j)) {
            UnCover(col(j));
        }
    }

    void ConnectColHeaders() {
        // Connect root node.
        m_RootNode = 0;
        right(m_RootNode) = 1;                //&m_Headers[0];
        left(right(m_RootNode)) = m_RootNode; // &m_RootNode;
        left(m_RootNode) = m_NumConstraints;  // &m_Headers[m_NumConstraints - 1];
        right(left(m_RootNode)) = m_RootNode; // &m_RootNode;

        for (int i = 1; i <= m_NumConstraints; ++i) {
            // Add rightwards connection
            if (i < m_NumConstraints) {
                right(i) = i + 1;
                left(right(i)) = i;
            }

            // Connect up/down to self
            up(i) = i;
            down(i) = i;

            // Initialize header fields
            count(i) = 0;
        }

        // Update node count
        m_NodeCount = m_NumConstraints + 1;
    }

    int Possibility(int row, int col, int num) const { return (row * m_Width * m_Width) + (col * m_Width) + num; }

    void PopulateGraph() {
        std::array<size_t, k_MaxPosibilities> rightmostNodes{0};
        int constraint = 0;
        // RC
        for (int r = 0; r < m_Width; ++r) {
            for (int c = 0; c < m_Width; ++c) {
                for (int n = 0; n < m_Width; ++n) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // RN
        for (int r = 0; r < m_Width; ++r) {
            for (int n = 0; n < m_Width; ++n) {
                for (int c = 0; c < m_Width; ++c) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
        // CN
        for (int c = 0; c < m_Width; ++c) {
            for (int n = 0; n < m_Width; ++n) {
                for (int r = 0; r < m_Width; ++r) {
                    int possibility = Possibility(r, c, n);
                    CreateNewNode(possibility, constraint, rightmostNodes);
                }
                ++constraint;
            }
        }
    }

    void CreateNewNode(int possibility, int constraint, std::array<size_t, k_MaxPosibilities> &rightmostNodes) {
        size_t node = m_NodeCount++;

        // Insert node into column (at lowest position)
        col(node) = constraint + 1;
        down(node) = col(node);
        up(node) = up(col(node));
        up(down(node)) = node;
        down(up(node)) = node;
        ++count(col(node));

        // Connect node to neighbours left & right
        size_t rightmost = rightmostNodes[possibility];
        if (rightmost) {
            size_t leftmost = right(rightmost);
            left(node) = rightmost;
            right(node) = leftmost;
            left(leftmost) = node;
            right(rightmost) = node;
        } else {
            // No other entries on given row, connect to self
            left(node) = node;
            right(node) = node;
        }
        rightmostNodes[possibility] = node;
    }

private:
    const int m_Width;
    const int m_NumConstraints{3 * m_Width * m_Width};

    size_t m_RootNode;
    std::array<Header, k_MaxConstraints> m_Headers;
    std::array<Node, k_MaxNodes + 1 + k_MaxConstraints> m_Nodes;
    int m_NodeCount = 0;
    // std::vector<Node> m_Nodes;
};

#if defined(_WIN32)
int main() {
    int n = 9;

    for (int iter = 0; iter < 100; ++iter) {
        // std::vector<std::vector<int>> init_grid = {
        //     {1, 0, 0, 0, 0, 0, 0, 0, 0}, {0, 2, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 3, 0, 0, 0, 0, 0, 0},
        //     {0, 0, 0, 4, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 5, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 6, 0, 0, 0},
        //     {0, 0, 0, 0, 0, 0, 7, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 8, 0}, {0, 0, 0, 0, 0, 0, 0, 0, 9},
        // };

        std::vector<std::vector<int>> init_grid = {
            {0, 8, 7, 6, 0, 4, 3, 2, 1}, {8, 0, 0, 0, 0, 0, 0, 0, 2}, {7, 0, 0, 0, 0, 0, 0, 0, 3},
            {6, 0, 0, 5, 2, 3, 0, 0, 0}, {0, 0, 0, 3, 0, 2, 0, 0, 5}, {4, 0, 1, 2, 3, 0, 0, 0, 6},
            {3, 0, 0, 0, 0, 0, 0, 0, 7}, {2, 0, 0, 0, 0, 0, 0, 0, 8}, {1, 0, 3, 0, 5, 0, 7, 8, 0},
        };

        const auto time_start = std::chrono::high_resolution_clock::now();

        ConstraintMatrix cm(n);
        for (int y = 0; y < n; ++y) {
            // std::string row;
            // getline(std::cin, row);
            for (int x = 0; x < n; ++x) {
                // grid.Set(Position{ x, y }, row[x] - '0');
                if (init_grid[y][x] != 0) {
                    if (!cm.Set(x, y, init_grid[y][x])) {
                        // Invalid grid
                        std::cout << "Invalid grid...\n";
                        return -1;
                    }
                }
            }
        }

        std::cout << cm.Solutions() << '\n';

        const auto time_end = std::chrono::high_resolution_clock::now();
        const auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();
        std::cerr << "Time : " << time_ms << "ms\n";
    }

    return 0;
}

#else

int main() {
    int n;
    std::cin >> n;
    std::cin.ignore();

    ConstraintMatrix cm(n);

    for (int y = 0; y < n; ++y) {
        std::string row;
        getline(std::cin, row);
        for (int x = 0; x < n; ++x) {
            int val = row[x] - '0';
            if (val != 0) {
                if (!cm.Set(x, y, val)) {
                    // Invalid grid
                    std::cout << "0\n";
                    return 0;
                }
            }
        }
    }

    std::cout << cm.Solutions() << '\n';

    return 0;
}
#endif